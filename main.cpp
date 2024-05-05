/**
This application renders a textured mesh that was loaded with Assimp.
*/
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <memory>
#include <glad/glad.h>

#include "Mesh3D.h"
#include "Object3D.h"
#include "AssimpImport.h"
#include "Animator.h"
#include "ShaderProgram.h"


#include "Skeletal.h"
#include "SkeletalAnimator.h"
#include <algorithm>


#define PI glm::pi<float>()

/**
 * @brief Defines a collection of objects that should be rendered with a specific shader program.
 */
template<class T>
struct Scene {
	ShaderProgram defaultShader;
	std::vector<Object3D> objects;
	std::vector<Animator<T>> animators;
};

/**
 * @brief Constructs a shader program that renders textured meshes in the Phong reflection model.
 * The shaders used here are incomplete; see their source codes.
 * @return 
 */
ShaderProgram phongLighting() {
	ShaderProgram program;
	try {
		program.load("shaders/light_perspective.vert", "shaders/lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return program;
}

/**
 * @brief Constructs a shader program that renders textured meshes without lighting.
 */
ShaderProgram textureMapping() {
	ShaderProgram program;
	try {
		program.load("shaders/texture_perspective.vert", "shaders/texturing.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return program;
}

ShaderProgram sameColor() {
	ShaderProgram program;
	try {
		program.load("shaders/texture_perspective.vert", "shaders/same_color.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return program;
}


ShaderProgram skeletalShader() {
	ShaderProgram program;
	try {
		program.load("shaders/skeletal.vert", "shaders/lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return program;
}

ShaderProgram shadowMapShader() {
	ShaderProgram program;
	try {
		program.load("shaders/shadow_map.vert", "shaders/shadow_map.frag", "shaders/shadow_map.gs");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return program;
}


/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "baseTexture") {
	sf::Image i;
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}


// Shadow
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
GLuint depthMapFBO;
unsigned int depthCubemap;
ShaderProgram setUpShadow() {
	
	glGenFramebuffers(1, &depthMapFBO);
	// Create depth texture
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return shadowMapShader();
}

void setUpLight(ShaderProgram& program) {
	program.activate();
	program.setUniform("ambientColor", glm::vec3(1, 1, 1));
	program.setUniform("directionalColor", glm::vec3(1, 1, 1));
	program.setUniform("material", glm::vec4(0.2, 0.5, 1, 32));
	//program.setUniform("hasDirectionalLight", true);
	//program.setUniform("directionalLight", glm::vec3(0, 0, -1));


	program.setUniform("light_constant", 1.0f);
	// distance inf
	program.setUniform("light_linear", 0.0f);
	program.setUniform("light_quadratic", 0.0f);
	// distance 100
	//program.setUniform("light_linear", 0.045f);
	//program.setUniform("light_quadratic", 0.0075f);
	// distance 50
	//program.setUniform("light_linear", 0.09f);
	//program.setUniform("light_quadratic", 0.0032f);
}


void renderSkeletal(sf::RenderWindow& window, ShaderProgram& program, SkeletalObject& obj, std::vector<glm::mat4>& transforms) {
	program.activate();
	program.setUniform("skeletal", true);
	for (int i = 0; i < transforms.size(); ++i)
		program.setUniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
	obj.render(window, program);
	program.setUniform("skeletal", false);
}



Scene<Object3D> lightScene() {
	Texture tmp_texture;
	auto mesh = Mesh3D::cube(tmp_texture);
	auto light_cube = Object3D(std::vector<Mesh3D>{mesh});
	light_cube.move(glm::vec3(0, 5, 0));
	light_cube.grow(glm::vec3(1, 1, 1));
	std::vector<Object3D> objects;
	objects.push_back(std::move(light_cube));

	return Scene<Object3D>{
		sameColor(),
		std::move(objects),
	};
}

// line segment - circle intersect ------------------------------------------------------------------------------------------------
struct Circle {
	glm::vec2 center;
	float radius;
};

struct Wall {
	glm::vec2 start;
	glm::vec2 end;
	glm::vec2 normal;  // Assuming normal is a unit vector

	SkeletalObject wall;

	Wall(std::vector<Texture>& textures, glm::vec3 pos, glm::vec3 rot, float width, float height) {
		wall = SkeletalObject(std::vector<SkeletalMesh>{SkeletalMesh::square(textures)});
		
		wall.grow(glm::vec3(width, height, 1));
		wall.move(pos);
		wall.rotate(rot);

		start = glm::vec2(-0.5f, 0.0f);
		end = glm::vec2(0.5f, 0.0f);
		
		float c = rot.y;
		glm::mat2 rotationMatrix = glm::mat2(
			glm::cos(c), -glm::sin(c),
			glm::sin(c), glm::cos(c)
		);
		start = rotationMatrix * start;
		end = rotationMatrix * end;

		normal = glm::vec2(0, 1);
		normal = glm::rotate(normal, -c);

		start *= width;
		end *= width;

		start += glm::vec2(pos.x, pos.z);
		end += glm::vec2(pos.x, pos.z);

		//std::cout <<wall.getPosition()<<": " << start << " " << end << " " << normal << "\n";
	}
};

float pointLineSignedDistance(const glm::vec2& point, const glm::vec2& start, const glm::vec2& end, const glm::vec2& normal) {
	glm::vec2 pointVec = point - start;
	float signedDist = glm::dot(pointVec, normal);
	return signedDist;
}

void checkCollision(Circle& circle, const Wall& wall, glm::vec2* target = nullptr) {
	auto x = circle.center.x,
		y = circle.center.y;
	if ((x < std::min(wall.start.x, wall.end.x) || x > std::max(wall.start.x, wall.end.x))
		&& (y < std::min(wall.start.y, wall.end.y) || y > std::max(wall.start.y, wall.end.y))) {
		return;
	}

	float distance = pointLineSignedDistance(circle.center, wall.start, wall.end, wall.normal);
	if (!target) {
		if (distance <= circle.radius && distance >= 0) {
			float overlap = circle.radius - distance;
			if (overlap > 0) {
				circle.center += wall.normal * overlap;
			}
		}
	}
	else {
		auto a = glm::normalize(*target - circle.center);
		auto b = wall.normal;
		auto distant_target_wall = pointLineSignedDistance(*target, wall.start, wall.end, wall.normal);
		if (glm::dot(a,b) > 0 && distance <= circle.radius && distant_target_wall >= 0) {

			float overlap = circle.radius - distance;
			if (overlap > 0) {
				circle.center += wall.normal * overlap;
			}
		}
	}
}


// 3D ----------------------------------------------------------
//struct Sphere {
//	glm::vec3 center;
//	float radius;
//};
//
//struct Plane {
//	glm::vec3 point;  // A point on the plane
//	glm::vec3 normal; // Normal vector to the plane, must be a unit vector
//};
//
//float pointPlaneDistance(const glm::vec3& point, const glm::vec3& planePoint, const glm::vec3& normal) {
//	return glm::dot(point - planePoint, normal);
//}
//
//bool checkCollision(const Sphere& sphere, const Plane& plane) {
//	float distance = pointPlaneDistance(sphere.center, plane.point, plane.normal);
//	return distance <= sphere.radius; // The sphere collides if the distance is less than its radius
//}
//
//void resolveCollision(Sphere& sphere, const Plane& plane) {
//	float distance = pointPlaneDistance(sphere.center, plane.point, plane.normal);
//	float overlap = sphere.radius - distance;
//
//	if (overlap > 0) {
//		sphere.center += plane.normal * overlap; // Move the sphere out of the plane by the overlap amount
//	}
//}

// ---------------------------------------------------------------------------------------------------------------------


int main() {
	// Initialize the window and OpenGL.
	sf::ContextSettings Settings;
	Settings.depthBits = 24; // Request a 24 bits depth buffer
	Settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	Settings.antialiasingLevel = 2;  // Request 2 levels of antialiasing
	sf::RenderWindow window(sf::VideoMode{ 1200, 800 }, "SFML Demo", sf::Style::Resize | sf::Style::Close, Settings);
	gladLoadGL();
	glEnable(GL_DEPTH_TEST);

	//window.setFramerateLimit(100);

	//-----------------------------------------------------------------------------------------------------
	auto shadow_shader = setUpShadow();
	//-----------------------------------------------------------------------------------------------------

	ShaderProgram skeletal_shader = skeletalShader();

	auto perspective = glm::perspective(glm::radians(45.0), static_cast<double>(window.getSize().x) / window.getSize().y, 0.1, 100.0);
	skeletal_shader.activate();
	skeletal_shader.setUniform("projection", perspective);

	setUpLight(skeletal_shader);



	Skeletal vampire1_model("models/vampire/dancing_vampire.dae", true);
	SkeletalAnimation vampire1_dance("models/vampire/dancing_vampire.dae", &vampire1_model);
	SkeletalAnimator vampire1_animator(&vampire1_dance);
	auto& vampire1 = vampire1_model.getRoot();
	vampire1.grow(glm::vec3(1.3, 1.3, 1.3));
	vampire1.move(glm::vec3(0, 0, -8));
	vampire1.addTexture(loadTexture("models/vampire/textures/Vampire_normal.png", "normalMap"));



	//-----------------------------------------------------------------------------------------------------
	Skeletal skeletal_model("models/model.dae", true);
	SkeletalAnimation dance_animation("models/model.dae", &skeletal_model);
	SkeletalAnimator skeletal_animator(&dance_animation);


	auto& vampire = skeletal_model.getRoot();
	//vampire.move(glm::vec3(0, 0, 0));
	float_t vampire_scale = 0.2;
	vampire.grow(glm::vec3(vampire_scale, vampire_scale, vampire_scale));
	vampire.setMass(10);


	float_t vampire_height = 1;
	float_t vampire_velocity = 4;
	glm::vec3 vampire_forward = glm::vec3(0, 0, 1);

	
	glm::vec3 desired_direction;
	Animator<SkeletalObject> rotate_vampire;
	rotate_vampire.addAnimation(
		[&vampire, &vampire_forward, &desired_direction]() {
			auto a = vampire_forward;
			a.y = 0;
			a = glm::normalize(a);
			auto b = desired_direction;
			b.y = 0;
			b = glm::normalize(b);

			auto tmp = glm::dot(a, b);
			tmp = std::max(tmp, -1.0f);
			tmp = std::min(tmp, 1.0f);
			auto angle = acos(tmp);

			//std::cout <<a<<b << "\n";
			angle *= glm::cross(a, b).y >= 0? 1 : -1;
			vampire_forward = desired_direction;
			//std::cout << "---------------  " << glm::degrees(angle) << "\n";
			return std::make_unique<RotationAnimation<SkeletalObject>>(vampire, 0.15, glm::vec3(0, angle, 0));
		}
	);

	//-----------------------------------------------------------------------------------------------------

	//Animator<SkeletalObject> jump_vampire;
	//jump_vampire.addAnimation(
	//	[&vampire]() {
	//		return std::make_unique<TranslationAnimation<SkeletalObject>>(vampire, 0.4, glm::vec3(0, 2, 0));
	//	}
	//);
	//jump_vampire.addAnimation(
	//	[&vampire]() {
	//		return std::make_unique<TranslationAnimation<SkeletalObject>>(vampire, 0.4, glm::vec3(0, -2, 0));
	//	}
	//);


	//wall -------------------------------------------------------------------------------------------------
	std::vector<Texture> textures = {
		loadTexture("models/brick_wall/brickwall.jpg", "baseTexture"),
		loadTexture("models/brick_wall/brickwall_normal.jpg", "normalMap"),
	};
	auto mesh = SkeletalMesh::square(textures);
	auto ground = SkeletalObject(std::vector<SkeletalMesh>{mesh});
	ground.rotate(glm::vec3(-PI / 2, 0, 0));
	ground.grow(glm::vec3(20, 20, 20));

	

	std::vector<Wall> walls = {
		Wall(textures, glm::vec3(-10, 2.5, 0), glm::vec3(0, PI/2, 0), 20.0f, 5.0f),
		Wall(textures, glm::vec3(10, 2.5, 0), glm::vec3(0, -PI / 2, 0), 20.0f, 5.0f),
		Wall(textures, glm::vec3(0, 2.5, -10), glm::vec3(0, 0, 0), 20.0f, 5.0f),
	};




	// ---------------------------------------------------------------------------------------------------------------



	auto tiger = assimpLoad("models/tiger/scene.gltf", true);
	tiger.grow(glm::vec3(0.01, 0.01, 0.01));
	tiger.move(glm::vec3(2, 2, 4));


	

	// light source -----------------------------------------------------------
	auto light_scene = lightScene();
	auto light_cube = light_scene.objects[0];
	ShaderProgram& light_shader = light_scene.defaultShader;
	light_shader.activate();
	light_shader.setUniform("projection", perspective);
	light_shader.setUniform("color", glm::vec4(1, 1, 1, 1));

	// camera --------------------------------------------------------------------
	float radius = 5.0f;
	float azimuth = 0;   // Horizontal angle
	float elevation = 0; // Vertical angle
	auto target = vampire.getPosition();
	target.y += vampire_height;
	glm::vec3 camera_pos(
		target.x + radius * cos(elevation) * sin(azimuth),
		target.y + radius * sin(elevation),
		target.z + radius * cos(elevation) * cos(azimuth)
	);
	auto up_vector = glm::vec3(0, 1, 0);
	auto camera = glm::lookAt(camera_pos, target, up_vector);

	// ----------------------------------------------------------------------------------------------------------------------
	// Ready, set, go!
	//for (auto& animator : scene.animators) {
	//	animator.start();
	//}
	bool running = true;
	sf::Clock c;


	sf::Vector2i last_mouse_position = sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2);
	bool moving = false,
		move_left = false,
		move_right = false,
		move_forward = false,
		move_backward = false,
		jumping = false;
	auto last_gravity_time = c.getElapsedTime();

	auto last = c.getElapsedTime();
	while (running) {
		sf::Event ev;
		while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
				running = false;
			}
			else if (ev.type == sf::Event::KeyPressed)
			{
				moving = true;
				if (ev.key.code == sf::Keyboard::W) {
					move_forward = true;
				}
				if (ev.key.code == sf::Keyboard::S) {
					move_backward = true;
				}
				if (ev.key.code == sf::Keyboard::A) {
					move_left = true;
				}
				if (ev.key.code == sf::Keyboard::D) {
					move_right = true;
				}
				if (ev.key.code == sf::Keyboard::Space) {
					jumping = true;
				}
			}
			else if (ev.type == sf::Event::KeyReleased) {
				if (ev.key.code == sf::Keyboard::W) {
					move_forward = false;
				}
				if (ev.key.code == sf::Keyboard::S) {
					move_backward = false;
				}
				if (ev.key.code == sf::Keyboard::A) {
					move_left = false;
				}
				if (ev.key.code == sf::Keyboard::D) {
					move_right = false;
				}
				if (ev.key.code == sf::Keyboard::Space) {
					jumping = false;
				}
			}
		}
		
		auto now = c.getElapsedTime();
		auto diff = now - last;
		auto diffSeconds = diff.asSeconds();
		last = now;
		//std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;


		
		
		
		sf::Vector2i mouse_position = sf::Mouse::getPosition();
		
		if (window.getSize().x - mouse_position.x <= 1 || mouse_position.x <= 1) {
			sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, mouse_position.y));
			last_mouse_position = sf::Mouse::getPosition();
		}
		else {
			float_t delta_x = 1.0f * (mouse_position.y - last_mouse_position.y) / (window.getSize().y) * 3.14 / 2.0;
			float_t delta_y = -1.0f * (mouse_position.x - last_mouse_position.x) / (window.getSize().x) * 3.14 / 2.0;

			azimuth += delta_y;
			elevation += delta_x;
			elevation = std::max(-glm::half_pi<float>() + 0.01f, std::min(glm::half_pi<float>() - 0.01f, elevation));
			
			
			target = vampire.getPosition();
			target.y += vampire_height;
			camera_pos = glm::vec3(
				target.x + radius * cos(elevation) * sin(azimuth),
				std::max(0.2f, target.y + radius * sin(elevation) ),
				target.z + radius * cos(elevation) * cos(azimuth)
			);

			// intersect wall - cam ---------------------------------------------------------------------------------
			Circle cam_circle = { {camera_pos.x, camera_pos.z}, 0.2f };
			

			for (auto& wall : walls) {
				auto tmp = glm::vec2(target.x, target.z);
				checkCollision(cam_circle, wall, &tmp);
			}
			camera_pos.x = cam_circle.center.x;
			camera_pos.z = cam_circle.center.y;
			// ----------------------------------------------------------------------------------------------------


			camera = glm::lookAt(camera_pos, target, up_vector);

			last_mouse_position = mouse_position;
		}


		
		


		skeletal_shader.activate();
		skeletal_shader.setUniform("view", camera);
		skeletal_shader.setUniform("viewPos", camera_pos);
		

		




		//-------------------------------------------------------------------------------------------------------------------------------------
		
		
		if ((!move_left && !move_right && !move_forward && !move_backward) || jumping) {
			moving = false;
		}
		else {
			moving = true;
		}
		if (moving && vampire.getPosition().y == 0) {
			skeletal_animator.UpdateAnimation(diff.asSeconds());
		}
		else {
			skeletal_animator.resetAnimation();
		}
		auto vampire_transforms = skeletal_animator.GetFinalBoneMatrices();
		
		//if (moving && jump_vampire.finish()) {
		//	skeletal_animator.UpdateAnimation(diffSeconds);
		//}
		//else {
		//	skeletal_animator.resetAnimation();
		//}
		//if (jumping) {
		//	if (jump_vampire.finish()) {
		//		jump_vampire.start();
		//	}
		//}
		//jump_vampire.tick(diffSeconds);


		glm::vec3 forward_cam = target - camera_pos;
		forward_cam.y = 0;
		forward_cam = glm::normalize(forward_cam);
		glm::vec3 right_cam = glm::cross(forward_cam, up_vector);
		right_cam.y = 0;
		right_cam = glm::normalize(right_cam);


		desired_direction = glm::vec3(0);
		if (move_forward) {
			//vampire.move(forward_cam* vampire_velocity* diff.asSeconds());
			desired_direction += forward_cam;
		}
		else if (move_backward) {
			//vampire.move(-forward_cam * vampire_velocity * diff.asSeconds());
			desired_direction -= forward_cam;
		}
			
		if (move_right) {
			//vampire.move(right_cam* vampire_velocity* diff.asSeconds());
			desired_direction += right_cam;
		}
		else if (move_left) {
			//vampire.move(-right_cam * vampire_velocity * diff.asSeconds());
			desired_direction -= right_cam;
		}

		auto vampire_velocity_y = vampire.getVelocity().y;
		vampire.setVelocity(glm::vec3(0, vampire_velocity_y, 0) + desired_direction * vampire_velocity);

		if (desired_direction.x != 0 || desired_direction.z != 0) {
			if (rotate_vampire.finish()) {
				rotate_vampire.start();
			}
		}

		rotate_vampire.tick(diffSeconds);


		

		if ((now - last_gravity_time).asMilliseconds() > 1.0f) {
			vampire.addForce(glm::vec3(0, -9.8, 0) * vampire.getMass());
			last_gravity_time = now;

			if (jumping && vampire.getPosition().y == 0) {
				
				vampire.addForce(glm::vec3(0, 15000, 0));
				jumping = false;
			}
			//std::cout << vampire.getPosition() << "\n";
			vampire.tick(diffSeconds);
		}
		

		
		auto vampire_pos = vampire.getPosition();
		if (vampire_pos.y <= 0.0005) {
			vampire_pos.y = 0;
			vampire.setPosition(vampire_pos);
			auto vampire_vel = vampire.getVelocity();
			vampire_vel.y =  0.0;
			vampire.setVelocity(vampire_vel);
		}

		// character collide with wall ---------------------------------------------------------------------------------------------------------
		Circle vampire_circle = { {vampire_pos.x, vampire_pos.z}, 0.5f };
		for (auto& wall : walls) {
			checkCollision(vampire_circle, wall);
		}
		vampire_pos.x = vampire_circle.center.x;
		vampire_pos.z = vampire_circle.center.y;
		vampire.setPosition(vampire_pos);
		

		//-------------------------------------------------------------------------------------------------------------------------------------
		
		
		

		vampire1_animator.UpdateAnimation(diffSeconds);
		auto vampire1_transforms = vampire1_animator.GetFinalBoneMatrices();

		
		//-------------------------------------------------------------------------------------------------------------------------------------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float near_plane = 0.1f;
		float far_plane = 100.0f;
		//glm::mat4 shadowProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		auto lightPos = light_cube.getPosition();
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		shadow_shader.activate();
		for (unsigned int i = 0; i < 6; ++i)
			shadow_shader.setUniform("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		shadow_shader.setUniform("far_plane", far_plane);
		shadow_shader.setUniform("lightPos", lightPos);



		renderSkeletal(window, shadow_shader, vampire, vampire_transforms);
		renderSkeletal(window, shadow_shader, vampire1, vampire1_transforms);

		ground.render(window, shadow_shader);
		tiger.render(window, shadow_shader);

		for (auto& wall : walls) {
			wall.wall.render(window, shadow_shader);
		}





		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//----------------------------------------------------------------------------------------------------------------------
		glViewport(0, 0, window.getSize().x, window.getSize().y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skeletal_shader.activate();
		skeletal_shader.setUniform("lightPos", light_cube.getPosition());
		skeletal_shader.setUniform("far_plane", far_plane);

		// max number of textures = 3
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		skeletal_shader.setUniform("depthMap", 4);



		renderSkeletal(window, skeletal_shader, vampire, vampire_transforms);
		renderSkeletal(window, skeletal_shader, vampire1, vampire1_transforms);

		ground.render(window, skeletal_shader);
		tiger.render(window, skeletal_shader);


		for (auto& wall : walls) {
			wall.wall.render(window, skeletal_shader);
		}


		//-------------------------------------------------------------------------------------------------------------------------------------

		light_shader.activate();
		light_shader.setUniform("view", camera);
		for (auto& o : light_scene.objects) {
			o.render(window, light_shader);
		}

		//-------------------------------------------------------------------------------------------------------------------------------------

		

		window.display();
	}

	return 0;
}


