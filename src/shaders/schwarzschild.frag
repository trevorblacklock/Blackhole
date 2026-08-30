#version 460 core

out vec4 fragColor;

in vec3 tex;

uniform samplerCube skybox;

layout(std140, binding = 1) uniform blackholeBuffer {
    vec4 blackholePos;
    vec4 cameraPos;
};

struct posAndVel {
    vec3 pos;
    vec3 vel;
};

struct accAndVel {
    vec3 vel;
    vec3 acc;
};

const int maxSteps = 1000;
const float rk4scale = 1.0/6.0;

const float mass = 1.0;
const float a = 0.0;

accAndVel geodesic_acceleration(vec3 pos, vec3 vel) {
    // Calculate initial values used in gradients once for performance
    float r = length(pos);
    float r3 = r*r*r;
    float rs = 2.0*mass;
    float rootg = 1.0 + rs/(4.0*r);
    float g = rootg*rootg;
    float k1 = (1.0-rs/(4.0*r))/rootg;
    float k2 = g*g;

    // Calculate gradients
    vec3 grad_k1 = pos*rs/(2.0*r3*g);
    vec3 grad_k2 = -rs*g*rootg*pos/r3;

    // Sub into acceleration formula, the signs here are swapped since
    // we are effectively tracing backwards along the geodesic.
    vec3 a = (0.5*grad_k2/k2)-(grad_k1/k1)+(vel*dot(vel,grad_k2)/k2);
    vec3 v = vel;

    // Return negative acceleration, since we are tracing backwards along lights path.
    return accAndVel(v, a);
}

posAndVel rk4_step(posAndVel initial, float dt) {
    // Calculate all Runge-Kutta coefficients
    accAndVel y1 = geodesic_acceleration(initial.pos, initial.vel);
    accAndVel y2 = geodesic_acceleration(initial.pos+0.5*y1.vel*dt, initial.vel+0.5*y1.acc*dt);
    accAndVel y3 = geodesic_acceleration(initial.pos+0.5*y2.vel*dt, initial.vel+0.5*y2.acc*dt);
    accAndVel y4 = geodesic_acceleration(initial.pos+y3.vel*dt, initial.vel+y3.acc*dt);

    // Calculate the next steps
    initial.vel += rk4scale*dt*(y1.acc+2.0*y2.acc+2.0*y3.acc+y4.acc);
    initial.pos += rk4scale*dt*(y1.vel+2.0*y2.vel+2.0*y3.vel+y4.vel);

    // Return the newly calculated position and velocity
    return initial;
}

vec4 geodesic(posAndVel initial) {
    // Setup and Schwarzschild radius
    float rs = 2.0*mass;
    // Initialize the initial distance from the black hole
    float r0 = length(initial.pos);
    // Setup the dynamic max and min step size
    float dtmax = 0.5*rs;
    float dtmin = 0.001*rs;
    // Now set the initial step size as a minimum
    float dt = dtmin;
    // Set a tolerance to use as an exit condition
    float tol = 50.0*rs;

    // Loop through max number of steps
    for (int i = 0; i < maxSteps; ++i) {
        // Run an adaptive rk4
        initial = rk4_step(initial, dt);
        // Get distance from black hole
        float r1 = length(initial.pos);
        // Aproximate first derivative of radius
        float dr = r1-r0;

        // Check for a reason to discontinue the loop, these include
        // entering the Schwarschild radius, or being far away enough
        // to assume a flat metric, from which we can directly extrapolate
        // the skybox uv coordinate to draw.
        if (r1 <= rs) return vec4(0.0,0.0,0.0,1.0);
        if (r1 >= tol && dr > 0.0) return texture(skybox, initial.vel);

        // Get the normalized distance from the Schwarzschild radius
        float rn = (r1-rs)/rs;

        // Adjust the step size proportionally to the distance from the horizon.
        // Here we employ an offset sigmoid function to only reduce stepsize when
        // close to the horizon.
        dt = mix(dtmin, dtmax, 1.0/(1.0+10.0*exp(-rn+3.0)));

        // Set the previous distance and dr here
        r0 = r1;
    }
    // By default just sample the skybox
    return texture(skybox, initial.vel);
}

void main() {
    // Get ray direction from camera to fragment
    vec3 vel = normalize(tex);
    vec3 pos = cameraPos.xyz - blackholePos.xyz;
    posAndVel initial = posAndVel(pos, vel);

    // Solve geodesic equation numerically
    fragColor = geodesic(initial);
}
