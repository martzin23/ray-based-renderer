#version 430
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D color_buffer;
layout (binding = 1) uniform sampler2D heightmap;

struct Body {
    vec3 position;
    int type;
    vec3 size;
    int material;
};
struct Material {
    vec3 color;
    float emission;
    float roughness;
    float metallic;
};
struct Ray {
    vec3 origin;
    vec3 direction;
};
struct Data {
    vec3 position;
    bool collided;
    vec3 normal;
    int index;
    float dist;
};

#define MAX_BODIES 128
layout (std140, binding = 1) uniform UniformBlock {
    int temporal_counter;
    int downsample_factor;
    int body_count;
    int shader_mode;
    mat4 camera_rotation_matrix;
    vec3 camera_position;
    int sdf_type;
    int max_bounces;
    int max_marches;
    float fov;
    float epsilon;
	int custom_int;
	float custom_float1;
	float custom_float2;
	float march_multiplier;
    vec3 sun_direction;
	int detail;
    float focus_distance;
    float focus_strength;
    float custom_float23;
    float custom_float24;
    vec3 light_position; // unused
    float light_intensity; // unused
};
layout (std140, binding = 2) uniform BodyBlock {
    Body bodies[MAX_BODIES];
};
layout (std140, binding = 3) uniform MaterialBlock {
    Material materials[MAX_BODIES];
};

vec3 pathTrace(Ray camera_ray, uint pixel_seed);
Data rayTrace(Ray ray);
bool boxIntersection(Ray ray, Body box, inout float t, out vec3 outNormal);
bool sphereIntersection(Ray ray, Body sphere, inout float t);

Data rayMarch(Ray camera_ray);
vec3 derivateNormal(vec3 position, float epsilon);
float SDF(vec3 position);

void focusBlur(inout Ray ray, inout uint seed, const float range, const float strength);
vec3 skyValue(vec3 direction);
vec3 randomDirection(inout uint state);
float randomNormal(inout uint state);
float randomUniform(inout uint state);

//------------------------------------------------------------------

void main() {

    //--------------------ORIENTING-------------------------------------

    // if (temporal_counter > 1024 || temporal_counter > 1 && shader_mode == 1) return;
    ivec2 pixel_position = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screen_size = imageSize(color_buffer) / downsample_factor;
    if (pixel_position.x >= screen_size.x || pixel_position.y >= screen_size.y) return;

    vec2 screen_uv = vec2((2.0 * pixel_position.x - screen_size.x) / screen_size.x, (2.0 * pixel_position.y - screen_size.y) / screen_size.x);
    uint pixel_seed = pixel_position.y * screen_size.x + pixel_position.x + temporal_counter * 69420;
//    uint pixel_seed = temporal_counter;

    //-------------------CAMERA RAY-------------------------------------

    Ray camera_ray;
    camera_ray.origin = camera_position;
    camera_ray.direction = normalize((camera_rotation_matrix * vec4(1.0, -screen_uv.x * fov, screen_uv.y * fov, 1.0)).xyz);

    //-------------------CALCULATING COLOR-----------------------------

    vec3 output_color;
    if (shader_mode == 0) {
        Data data = rayTrace(camera_ray);
        Data shadow_data = rayTrace(Ray(data.position + epsilon * data.normal, sun_direction));

        vec3 color = materials[bodies[data.index].material].color;
        float diffuse = clamp(dot(data.normal, sun_direction) * 0.5 + 0.5, 0.0, 1.0);
        float shadow = clamp(float(!shadow_data.collided), 0.2, 1.0);

        output_color = mix(vec3(0.0), color * diffuse * shadow, data.collided);
//         output_color = vec3(bluenoise_value);
    }
    else if (shader_mode == 1) {
        camera_ray.direction = normalize(camera_ray.direction + randomDirection(pixel_seed) * 0.0005 * fov * downsample_factor);
        focusBlur(camera_ray, pixel_seed, focus_distance, focus_strength);

        vec3 current_color = pathTrace(camera_ray, pixel_seed);
        vec3 previous_color = imageLoad(color_buffer, pixel_position).xyz;
        output_color = previous_color * ((temporal_counter - 1.0) / temporal_counter) + current_color * (1.0 / temporal_counter);
    }
    else if (shader_mode == 2) {
        camera_ray.direction = normalize(camera_ray.direction + randomDirection(pixel_seed) * 0.0003 * fov * downsample_factor);
        focusBlur(camera_ray, pixel_seed, focus_distance, focus_strength);

        Data data = rayMarch(camera_ray);

        vec3 current_color = vec3(1.0 - data.index/(max_marches * 1.0));
        vec3 previous_color = imageLoad(color_buffer, pixel_position).xyz;
        if (isinf(previous_color).x || isnan(previous_color).x) previous_color = vec3(0.0);
        output_color = previous_color * ((temporal_counter - 1.0) / temporal_counter) + current_color * (1.0 / temporal_counter);
    }
    else if (shader_mode == 3) {
        camera_ray.direction = normalize(camera_ray.direction + randomDirection(pixel_seed) * 0.0003 * fov * downsample_factor);
        focusBlur(camera_ray, pixel_seed, focus_distance, focus_strength);

        Data data = rayMarch(camera_ray);
        Data shadow_data = rayMarch(Ray(data.position + pow(2,-detail) * 2 * data.normal, sun_direction));
        
        const vec3 color = -data.normal * 0.25 + 0.75;
        const float shadow = shadow_data.dist < 10.0 ? 0.0 : 1.0;
        const float diffuse = clamp(dot(sun_direction, data.normal), 0.0, 1.0);
        const float specular = pow(clamp(dot(sun_direction, reflect(camera_ray.direction, data.normal)),0.0, 1.0), 30);
        const vec3 sky = skyValue(camera_ray.direction);
        
        const vec3 current_color = (data.dist > 10.0) ? sky : (color + specular) * (shadow * diffuse + 0.1);
        vec3 previous_color = imageLoad(color_buffer, pixel_position).xyz;
        if (isinf(previous_color).x || isnan(previous_color).x) previous_color = vec3(0.0);
        output_color = previous_color * ((temporal_counter - 1.0) / temporal_counter) + current_color * (1.0 / temporal_counter);
    }
    else {

//        camera_ray.direction = normalize(camera_ray.direction + randomDirection(pixel_seed) * 0.0003 * fov * downsample_factor);
//        focusBlur(camera_ray, pixel_seed, focus_distance, focus_strength);
//
//        Data data = rayMarch(camera_ray);
//        const vec3 ld = light_position - data.position;
//        Data shadow_data = rayMarch(Ray(data.position + pow(2,-detail) * 2 * data.normal, normalize(ld)));
//        
//        const vec3 color = mix(-data.normal * 0.25 + 0.75, vec3(1, 0.561, 0), 0.25);
//        const float shadow = (shadow_data.dist < length(ld)) ? 0.0 : light_intensity / length(ld);
//        const float diffuse = clamp(dot(normalize(ld), data.normal), 0.0, 1.0);
//        const float specular = pow(clamp(dot(normalize(ld), reflect(camera_ray.direction, data.normal)),0.0, 1.0), 30);
//        const float ao = clamp(1.0 - 1.0 * data.index / max_marches, 0.5, 1.0);
//        
//        const vec3 current_color = (color + specular) * (shadow * diffuse + 0.1) * ao * int(data.dist < 10.0);
//        vec3 previous_color = imageLoad(color_buffer, pixel_position).xyz;
//        if (isinf(previous_color).x || isnan(previous_color).x) previous_color = vec3(0.0);
//        output_color = previous_color * ((temporal_counter - 1.0) / temporal_counter) + current_color * (1.0 / temporal_counter);

//        
        camera_ray.direction = normalize(camera_ray.direction + randomDirection(pixel_seed) * 0.0003 * fov * downsample_factor);
        focusBlur(camera_ray, pixel_seed, focus_distance, focus_strength);

        Ray ray = camera_ray;
        vec3 sample_color = vec3(0.0);
        vec3 ray_color = vec3(1.0);

        for (int bounce_counter=0; bounce_counter < max_bounces; bounce_counter++) {
            Data data = rayMarch(ray);

            if (!data.collided) {
                if (data.dist > 10 || bounce_counter == 0) {
                    vec3 sky = skyValue(ray.direction);
                    sample_color += sky * ray_color;
                }
                else {
                    sample_color += 0 * ray_color;
                }
                break;
            }
            
            ray.direction = normalize(randomDirection(pixel_seed) + data.normal);
            ray.origin = data.position + ray.direction * epsilon;
            
            vec3 color = vec3(1.0);
//            vec3 color = -data.normal * 0.25 + 0.75;
            vec3 emission = vec3(0.0);
            sample_color += emission * color * ray_color;
            ray_color *= color;
        }

        vec3 current_color = sample_color;
        vec3 previous_color = imageLoad(color_buffer, pixel_position).xyz;
        if (isinf(previous_color).x || isnan(previous_color).x) previous_color = vec3(0.0);
        output_color = previous_color * ((temporal_counter - 1.0) / temporal_counter) + current_color * (1.0 / temporal_counter);

    }
    imageStore(color_buffer, pixel_position, vec4(output_color, 1.0));
}

//------------------------------------------------------------------

vec3 pathTrace(Ray camera_ray, uint pixel_seed) {
    Ray ray = camera_ray;
    vec3 sample_color = vec3(0.0);
    vec3 ray_color = vec3(1.0);

    for (int bounce_counter=0; bounce_counter < max_bounces; bounce_counter++) {
        Data data = rayTrace(ray);

        if (!data.collided) {
            vec3 sky = skyValue(ray.direction);
            sample_color += sky * ray_color;
            break;
        }
            
        Material material = materials[bodies[data.index].material];

        if (material.roughness == 0.0) {
            vec3 reflect_direction = reflect(ray.direction, data.normal);
            ray.direction = normalize(randomDirection(pixel_seed) + data.normal);
            ray.direction = normalize(mix(ray.direction, reflect_direction, material.metallic));
        } else {
            ray.direction = normalize(refract(-ray.direction, data.normal, material.roughness));
        }
        ray.origin = data.position + ray.direction * epsilon;

        sample_color += material.emission * material.color * ray_color;
        ray_color *= material.color;
    }

    return sample_color;
}

Data rayTrace(Ray ray) {
    Data data;
    data.collided = false;
    data.index = 0;
    float min_t = 99999;
    vec3 box_normal;

    for (int i=0; i<body_count; i++) {
        float t;
        bool local_hit = (bodies[i].type == 0) ? sphereIntersection(ray, bodies[i], t) : boxIntersection(ray, bodies[i], t, box_normal);
        data.collided = data.collided || local_hit;

        if (local_hit && t < min_t) {
            min_t = t;
            data.index = i;
        }
    }
    
    data.position = ray.origin + min_t * ray.direction;
    data.dist = min_t;
    boxIntersection(ray, bodies[data.index], min_t, box_normal);
    data.normal = (bodies[data.index].type == 0) ? normalize(data.position - bodies[data.index].position) : box_normal;
    return data;
};

bool sphereIntersection(Ray ray, Body sphere, inout float t) {
    // https://www.youtube.com/@GetIntoGameDev
    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.size.x * sphere.size.x;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float tN = (-b - sqrt(discriminant)) / (2 * a);
//    float tF = (-b + sqrt(discriminant)) / (2 * a);
    t = tN;
    return (tN > 0);


//    vec3 oc = ray.origin - sphere.position;
//    float b = dot( oc, ray.direction );
//    float c = dot( oc, oc ) - sphere.size.x * sphere.size.x;
//    float h = b*b - c;
//    if (h < 0.0) return false;
//    h = sqrt( h );
//    if (-b -h < 0.0) t = -b + h;
//    else t = -b - h;
//    return (-b + h) > 0.0;
}

bool boxIntersection(Ray ray, Body box, inout float t , out vec3 outNormal) {
    // https://iquilezles.org/articles/intersectors/
    vec3 m = 1.0/ray.direction;
    vec3 n = m*(ray.origin - box.position);
    vec3 k = abs(m)*box.size;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if(tN>tF || tF<0.0) return false;

    outNormal = (tN>0.0) ? step(vec3(tN),t1) : -step(t2,vec3(tF));
    outNormal *= -sign(ray.direction);

    t = min(tN, tF);
    return true;
}

//------------------------------------------------------------------

Data rayMarch(Ray camera_ray) {
    Data data;
    data.collided = false;

    float total_dist = 0;
    for (data.index=0; data.index<max_marches; data.index++) {
        float d = SDF(camera_ray.origin);
        if (d < pow(2,-detail)) {
            data.collided = true;
            break;
        }
        else if (d > 100) {
            data.index = max_marches;
            break;
        }
        camera_ray.origin += camera_ray.direction * d * march_multiplier;
        total_dist += d * march_multiplier;
    }

    data.normal = derivateNormal(camera_ray.origin, epsilon);
    data.position = camera_ray.origin;
    data.dist = total_dist;
    return data;
}

vec3 derivateNormal(vec3 position, float epsilon) {
    vec3 normal;
	normal.x = (SDF(position + vec3(epsilon, 0.0, 0.0)) - SDF(position - vec3(epsilon, 0.0, 0.0))) / (2 * epsilon);
	normal.y = (SDF(position + vec3(0.0, epsilon, 0.0)) - SDF(position - vec3(0.0, epsilon, 0.0))) / (2 * epsilon);
	normal.z = (SDF(position + vec3(0.0, 0.0, epsilon)) - SDF(position - vec3(0.0, 0.0, epsilon))) / (2 * epsilon);
    return normalize(normal);
}

float SDF(vec3 p) {
    if (sdf_type == 1) {

        // https://www.youtube.com/@SebastianLague
        vec3 z = p;
        float dr = 1;
        float r;
        float power = custom_float1;

        for (int i=0; i<detail; i++) {
            r = length(z);
            if (r > 2) break;

            float theta = acos(z.z / r) * power;
            float phi = atan(z.y, z.x) * power;
            float zr = pow(r, power);
            dr = pow(r, power - 1) * power * dr + 1;
            z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
            z += p;
        }
        return 0.5 * log(r) * r / dr;

    }
    else if (sdf_type == 2) {

        // http://blog.hvidtfeldts.net/index.php/2011/11/distance-estimated-3d-fractals-vi-the-mandelbox/
        // scale = -1.75, folding limit = 1, folding value = 2, fixed radius = 1.0, and min radius = 0.5
        const float scale = custom_float2;
        const float folding_limit = custom_float23;
        const float min_radius2 = custom_float24;
        const float fixed_radius2 = custom_float1;
        vec3 z = p;
        float dr = 1.0;
        for (int n = 0; n < detail + 2; n++) {
	        z = clamp(z, -folding_limit, folding_limit) * 2.0 - z;

	        float r2 = dot(z,z);
	        if (r2 < min_radius2) { 
		        float temp = fixed_radius2 / min_radius2;
		        z *= temp;
		        dr *= temp;
	        } else if (r2 < fixed_radius2) { 
		        float temp = fixed_radius2 / r2;
		        z *= temp;
		        dr *= temp;
	        }
            z = scale * z + p;  
            dr = dr * abs(scale) + 1.0;
        }
        float r = length(z);
        return r / abs(dr);

    }
    else if (sdf_type == 3) {

        float min_dist = 1000;
        for (int i=0; i<body_count; i++) {
            Body body = bodies[i];
            float curr_dist;

            if (body.type == 0) {
                curr_dist = length(p - body.position) - body.size.x;
            } else {
                vec3 q = abs(p - body.position) - body.size;
                curr_dist = length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
            }
            min_dist = min(min_dist, curr_dist);
        }
        return min_dist;

    }
    else if (sdf_type == 4) {
        ivec2 texSize = textureSize(heightmap, 0);
        vec2 texCoord = p.xy / texSize * 200 / custom_float2;
        float height = texture(heightmap, texCoord).x;
        return p.z - height * custom_float1;
    }
    else {
//        return length(position) - 1;
//        return position.z + sin(position.y);
//        float x = floor(sin(position.x) * 10.0) / 10.0;
//        float y = floor(sin(position.y) * 10.0) / 10.0;
//        return position.z + x + y;


//        // https://github.com/Angramme/fractal_viewer/blob/master/fractals/koch_curve.glsl
        const float PI = 3.14 * custom_float1;
        const mat2 rot60deg = mat2(cos(PI/3), -sin(PI/3), sin(PI/3), cos(PI/3));
        const mat2 rotm60deg = mat2(cos(PI/3), sin(PI/3), -sin(PI/3), cos(PI/3));

        float s2 = 1.;
        for(int i=0; i<detail; i++){
            const float X1 = 2./3;
            s2 *= X1;
            p /= X1;
            if(abs(p.z) > -p.x*1.73205081){
                p.x *= -1;
                p.xz = (p.z > 0 ? rotm60deg : rot60deg) * p.xz;
            }
            p.zy = p.yz;
            p.x += 1. * custom_float2;
        }

        if(abs(p.z) > p.x*1.73205081){
            p.x *= -1;
            p.xz = (p.z > 0 ? rot60deg : rotm60deg) * p.xz;
        }
        const float X2 = 1.15470053839;
        float D = abs(p.y)+X2*p.x-X2;
        const float X1 = 1/sqrt(1+X2*X2);
        D *= X1;
        return D * s2;


        // https://github.com/adamsol/FractalView/blob/master/src/renderers/fractal/juliabulb.glsl
//        const int NUM_ITERATIONS = detail;
//        const float EXPONENT = custom_float1;
//        vec3 z = p;
//        vec3 d = vec3(1.0);
//        float r = 0.0;
//        float b = 10000.0;
//
//        for (int i = 0; i < NUM_ITERATIONS; ++i) {
//	        d = EXPONENT * pow(r, EXPONENT-1.0) * d + 1.0;
//	        if (r > 0.0) {
//		        float phi = atan(z.z, z.x);
//		        phi *= EXPONENT;
//		        float theta = acos(z.y/r);
//		        theta *= EXPONENT;
//		        r = pow(r, EXPONENT);
//		        z = vec3(cos(phi) * cos(theta), sin(theta), sin(phi) * cos(theta)) * r;
//	        }
//	        z += vec3(0.3, -0.9, -0.2);
//	        r = length(z);
//	        b = min(r, b);
//	        if (r >= 2.0)
//		        break;
//        }
//        return r * log(r) * 0.5 / length(d);

    }
}

//------------------------------------------------------------------

float randomUniform(inout uint state) {
    // https://www.youtube.com/@SebastianLague
    state *= (state + 195439) * (state + 124395) * (state + 845921);
    return state / 4294967295.0;
}

float randomNormal(inout uint state) {
    // https://stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * randomUniform(state);
    float rho = sqrt(-2 * log(randomUniform(state)));
    return rho * cos(theta);
}

vec3 randomDirection(inout uint state) {
    return normalize(vec3(randomNormal(state), randomNormal(state), randomNormal(state)));
}

void focusBlur(inout Ray ray, inout uint seed, const float range, const float strength) {
    vec3 focus_point = ray.origin + ray.direction * range;
    ray.origin += randomDirection(seed) * strength;
    ray.direction = normalize(focus_point - ray.origin);
}

vec3 skyValue(vec3 direction) {
    const vec3 sun_color = vec3(1.0, 1.0, 0.8);
    const float sun_intensity = 100.0;

    const vec3 horizon_color = vec3(1.8, 1.8, 2.0);
    const vec3 zenith_color = vec3(0.2, 0.2, 0.8);
    const vec3 ground_color = vec3(0.1);
    const float sky_intensity = 0.5;

    float altitude = dot(direction, vec3(0.0, 0.0, 1.0));
    float day_factor = dot(vec3(0.0, 0.0, 1.0), sun_direction) * 0.5 + 0.5;
    float sun_factor = pow(dot(direction, sun_direction) * 0.5 + 0.5, 100.0 + (1 - day_factor) * 1000.0);

    vec3 day_color = mix(horizon_color, zenith_color, pow(clamp(abs(altitude), 0.0, 1.0), 0.5)) * sky_intensity;
    vec3 night_color = day_color * vec3(0.1, 0.1, 0.3);
    vec3 sun_value = sun_factor * sun_color * sun_intensity;

    return mix(night_color, day_color, day_factor) + sun_value * day_factor;
}

//------------------------------------------------------------------

