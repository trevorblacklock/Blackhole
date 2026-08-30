#version 460 core

out vec4 fragColor;

in vec3 tex;

uniform samplerCube skybox;

layout(std140, binding = 1) uniform blackholeBuffer {
    vec4 blackholePos;
    vec4 cameraPos;
    float time;
};

struct packedResult {
    vec4 pos;
    vec4 p;
};

struct packedGrad {
    vec4 p;
    vec4 pdot;
};

const float epsilon = 1e-3;
const mat4 perturbation = mat4(epsilon);
const mat4 minkowski = mat4(
        -1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

const float pi = 3.14159265359;

const int maxSteps = 2000;
const float rk4scale = 1.0 / 6.0;

const float rs = 2.0;
const float a = 0.99;
const float a2 = a * a;

const float z1 = 1.0 + pow(1.0 - a2, 1.0 / 3.0) * (pow(1.0 + a, 1.0 / 3.0) + pow(1.0 - a, 1.0 / 3.0));
const float z2 = sqrt(3.0 * a2 + z1 * z1);
const float rIsco = 3.0 + z2 - sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2));

const float kappa = 0.1;

float r_coeff(vec3 pos) {
    float R2a2 = dot(pos, pos) - a2;
    return sqrt(0.5 * (R2a2 + sqrt(R2a2 * R2a2 + 4.0 * a2 * pos.y * pos.y)));
}

float f_coeff(float r, float z) {
    float r3 = r * r * r;
    return rs * r3 / (r3 * r + a * a * z * z);
}

vec4 k_coeff(float r, vec3 pos) {
    float denom = r * r + a * a;
    return vec4(1.0, (r * pos.x + a * pos.z) / denom, pos.y / r, (r * pos.z - a * pos.x) / denom);
}

mat4 kerr(vec3 pos) {
    float r = r_coeff(pos);
    float f = f_coeff(r, pos.y);
    vec4 k = k_coeff(r, pos);
    return minkowski + f * mat4(k.x * k, k.y * k, k.z * k, k.w * k);
}

mat4 kerr_inverse(vec3 pos) {
    float r = r_coeff(pos);
    float f = f_coeff(r, pos.y);
    vec4 k = minkowski * k_coeff(r, pos);
    return minkowski - f * mat4(k.x * k, k.y * k, k.z * k, k.w * k);
}

float hamiltonian(vec4 pos, vec4 p) {
    // Calculate the Hamiltonian using the momentum and inverse metric
    return 0.5 * dot(kerr_inverse(pos.yzw) * p, p);
}

float disk_density(vec3 pos) {
    float r = r_coeff(pos);
    float theta = acos(pos.y / r);
    return 0.0;
}

packedGrad hamiltonian_gradient(vec4 pos, vec4 p) {
    // Approximate the gradient with a small perturbation
    float ham0 = hamiltonian(pos, p);
    vec4 ham1 = vec4(hamiltonian(pos + perturbation[0], p),
            hamiltonian(pos + perturbation[1], p),
            hamiltonian(pos + perturbation[2], p),
            hamiltonian(pos + perturbation[3], p));

    vec4 pdot = (ham1 - ham0) / epsilon;
    return packedGrad(p, pdot);
}

packedResult rk4_step(packedResult initial, float dt) {
    // Calculate all Runge-Kutta coefficients
    mat4 g = kerr_inverse(initial.pos.yzw);
    packedGrad k1 = hamiltonian_gradient(initial.pos, initial.p);
    vec4 v1 = g * k1.p;
    packedGrad k2 = hamiltonian_gradient(initial.pos + 0.5 * v1 * dt, initial.p + 0.5 * k1.pdot * dt);
    vec4 v2 = g * k2.p;
    packedGrad k3 = hamiltonian_gradient(initial.pos + 0.5 * v2 * dt, initial.p + 0.5 * k2.pdot * dt);
    vec4 v3 = g * k3.p;
    packedGrad k4 = hamiltonian_gradient(initial.pos + v3 * dt, initial.p + k3.pdot * dt);
    vec4 v4 = g * k4.p;

    // Calculate the next steps, firstly combining momentum to accuractely find velocity
    initial.pos += rk4scale * dt * (v1 + 2.0 * v2 + 2.0 * v3 + v4);
    initial.p -= rk4scale * dt * (k1.pdot + 2.0 * k2.pdot + 2.0 * k3.pdot + k4.pdot);

    // Return the final approximation
    return initial;
}

vec3 momentum_to_velocity(packedResult ray) {
    return (kerr_inverse(ray.pos.yzw) * ray.p).yzw;
}

vec4 get_initial_momentum(vec3 pos, vec3 v) {
    // Determine the value of tdot to ensure light-like geodesic.
    // This solution is only valid when f < 1, i.e outside horizon.
    float r = r_coeff(pos);
    float f = f_coeff(r, pos.y);
    vec4 k = k_coeff(r, pos);
    float kdotv = dot(k.yzw, v);
    float tdot = (-f * kdotv - sqrt(f * kdotv * kdotv - f + 1)) / (f - 1);
    return kerr(pos) * vec4(tdot, v);
}

vec4 geodesic(packedResult initial) {
    // Initialize the initial distance from the black hole
    float r0 = r_coeff(initial.pos.yzw);
    // Setup the dynamic max and min step size
    float dtmax = 2.0 * rs;
    float dtmin = 0.001 * rs;
    // Now set the initial step size as a minimum
    float dt = dtmin;
    // Set a tolerance to use as an exit condition
    float tol = 50.0 * rs;
    // Calculate the outer horizon
    float horizon = (rs + sqrt(rs * rs - 4 * a * a)) / 2;
    // Optical depth
    float tau = 0.0;
    vec3 emission = vec3(0.0);

    // Loop through max number of steps
    for (int i = 0; i < maxSteps; ++i) {
        // Run an adaptive rk4
        initial = rk4_step(initial, dt);
        // Get distance from black hole
        float r1 = r_coeff(initial.pos.yzw);
        // Aproximate first derivative of radius
        float dr = r1 - r0;

        // Check for a reason to discontinue the loop, these include
        // entering the Schwarschild radius, or being far away enough
        // to assume a flat metric, from which we can directly extrapolate
        // the skybox uv coordinate to draw.
        if (r1 <= horizon) return vec4((1.0 - exp(-kappa * tau)) * emission, 1.0);
        if (r1 >= tol && dr > 0.0) break;

        // Get the normalized distance from the Schwarzschild radius
        float rn = (r1 - horizon) / horizon;

        // Adjust the step size proportionally to the distance from the horizon.
        // Here we employ an offset sigmoid function to only reduce stepsize when
        // close to the horizon.
        dt = mix(dtmin, dtmax, 1.0 / (1.0 + 10.0 * exp(-rn + 3.0)));

        // Set the previous radius and timestep here
        r0 = r1;
    }
    // By default just sample the skybox
    float transmission = exp(-kappa * tau);
    return vec4(texture(skybox, momentum_to_velocity(initial)).rgb * transmission + (1.0 - transmission) * emission, 1.0);
}

void main() {
    // Get ray direction from camera to fragment
    vec3 vel = normalize(tex);
    vec3 pos = cameraPos.xyz - blackholePos.xyz;
    packedResult initial = packedResult(vec4(0.0, pos), get_initial_momentum(pos, vel));

    // Solve geodesic equation numerically
    fragColor = geodesic(initial);
}
