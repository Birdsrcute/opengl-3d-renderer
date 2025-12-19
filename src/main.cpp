/**
* Starter code for CECS 449 final project.
* Major modules:
*	Mesh: an OpenGL VAO associated with one or more textures. Transfers vertices from RAM to VRAM. (INCOMPLETE)
*   Object3D: pairs a Mesh with world-space parameters (position, orientation, scale), and methods to manipulate
*			  the object in world space.
*			  Object3D is hierarchical; a "hierarchical mesh" is actually a tree of Object3Ds, which render
*			  recursively. (INCOMPLETE)
*   AssimpImport: loads an assimp model file into an hierarchical Object3D. (INCOMPLETE)
*   Animator: a list of animations to apply over given time intervals.
*   Main: initializes a Scene, advances Animators, and renders objects in the scene.
*/
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include <numbers>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Graphics.hpp>

#include "AssimpImport.h"
#include "Mesh.h"
#include "SceneObject.h"
#include "ShaderProgram.h"

#define M_PI std::numbers::pi_v<float>

// We use a structure to track all the elements of a scene, including a list of objects,
// a list of animators, and a shader program to use to render those objects.
struct Scene {
	ShaderProgram program{};
	std::vector<SceneObject> objects{};
};

/**
 * @brief Constructs a shader program that applies the Phong reflection model.
 */
ShaderProgram phongLightingShader() {
	ShaderProgram shader{};
	try {
		// These shaders are INCOMPLETE.
		shader.load("shaders/light_perspective.vert", "shaders/lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Constructs a shader program that performs texture mapping with no lighting.
 */
ShaderProgram texturingShader() {
	ShaderProgram shader{};
	try {
		shader.load("shaders/texture_perspective.vert", "shaders/texturing.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

ShaderProgram depthShader() {
	ShaderProgram shader{};
	try {
		shader.load("shaders/shadow_shader.vert", "shaders/shadow_shader.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

ShaderProgram clipSpaceNoLightingProgramShader() {
	ShaderProgram shader{};
	try {
		shader.load("shaders/minimap_clip.vert", "shaders/minimap_circle.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "baseTexture") {
	StbImage i{};
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}

/*****************************************************************************************
*  DEMONSTRATION SCENES
*****************************************************************************************/
Scene bunny() {
	Scene scene{ texturingShader() };

	// We assume that (0,0) in texture space is the upper left corner, but some artists use (0,0) in the lower
	// left corner. In that case, we have to flip the V-coordinate of each UV texture location. The last parameter
	// to assimpLoad controls this. If you load a model and it looks very strange, try changing the last parameter.
	auto bunny{ assimpLoad("models/bunny_textured.obj", true) };
	bunny.scale = glm::vec3{ 9, 9, 9 };
	bunny.position = glm::vec3{ 0.2, -1, 0 };

	// Move all objects into the scene's objects list.
	scene.objects.push_back(std::move(bunny));
	return scene;
}


/**
 * @brief Demonstrates loading a square, oriented as the "floor", with a manually-specified texture
 * that does not come from Assimp.
 */
Scene marbleSquare() {
	Scene scene{ texturingShader() };

	std::vector<Texture> textures{
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "baseTexture"),
	};
	auto mesh{ Mesh::square(textures) };
	SceneObject floor;
	floor.meshes.push_back(mesh);
	floor.scale = glm::vec3{ 5, 5, 5 };
	floor.position = glm::vec3{ 0, -1.5, 0 };
	floor.orientation = glm::vec3{ -M_PI / 2, 0, 0 };

	scene.objects.push_back(std::move(floor));
	return scene;
}

/**
 * @brief Loads a cube with a cube map texture.
 */
Scene cube() {
	Scene scene{ texturingShader() };
	auto cube{ assimpLoad("models/cube.obj", true) };
	scene.objects.push_back(std::move(cube));
	return scene;
}

/**
 * @brief Constructs a scene of a tiger sitting in a boat, where the tiger is the child object
 * of the boat.
 * @return
 */
Scene lifeOfPi() {
	// This scene is more complicated; it has child objects, as well as animators.
	Scene scene{ texturingShader() };

	auto boat{ assimpLoad("models/boat/boat.fbx", true) };
	boat.position = glm::vec3{ 0, -0.7, 0 };
	boat.scale = glm::vec3{ 0.01, 0.01, 0.01 };
	
	auto tiger{ assimpLoad("models/tiger/scene.gltf", true) };
	boat.children.push_back(std::move(tiger));

	// Move the boat into the scene list.
	scene.objects.push_back(std::move(boat));
	//scene.objects.push_back(std::move(tiger));
	return scene;
}

Scene lovebird() {
	Scene scene{ texturingShader() };

	auto lovebird{ assimpLoad("models/lovebird/lovebird_body.fbx", true) };
	lovebird.position = glm::vec3{ 0, -0.7, 0 };
	lovebird.scale = glm::vec3{ 0.0001, 0.0001, 0.0001 };

	auto wing_L{ assimpLoad("models/lovebird/lovebird_wing_L.fbx", true) };
	lovebird.children.push_back(std::move(wing_L));

	auto wing_R{ assimpLoad("models/lovebird/lovebird_wing_R.fbx", true) };
	lovebird.children.push_back(std::move(wing_R));

	// Move the boat into the scene list.
	scene.objects.push_back(std::move(lovebird));
	//scene.objects.push_back(std::move(tiger));
	return scene;
}

Scene projectScene() {
	Scene scene{ phongLightingShader() };

	float s = 0.01f;

	auto lovebird{ assimpLoad("models/lovebird/lovebird_body.fbx", true) };
	lovebird.position = glm::vec3{ 0, 0, 0 };
	lovebird.scale = glm::vec3{ s, s, s };

	auto hinge_wing_L{ assimpLoad("models/lovebird/hinge_wing_L.fbx", true) };
	hinge_wing_L.center = glm::vec3{ 75.0f, 250.0f, 65.0f };
	lovebird.children.push_back(std::move(hinge_wing_L));
	

	auto hinge_wing_R{ assimpLoad("models/lovebird/hinge_wing_R.fbx", true) };
	hinge_wing_R.center = glm::vec3{ -75.0f, 250.0f, 65.0f };
	lovebird.children.push_back(std::move(hinge_wing_R));

	/*
	auto test{ assimpLoad("models/cube.obj", true) };
	
	test.scale = glm::vec3{ 50.0f, 50.0f, 50.0f };
	test.position = glm::vec3{ 75.0f, 250.0f, 65.0f };      // start at bird origin
	lovebird.children.push_back(std::move(test));
	*/

	// Move the lovebird into the scene list.
	scene.objects.push_back(std::move(lovebird));

	// load city
	auto city{ assimpLoad("models/city/city_scaled.fbx", true) };
	city.position = glm::vec3{ 0, -0.7, 0 };
	city.scale = glm::vec3{ 0.04, 0.04, 0.04 };

	// Move the city into the scene list.
	scene.objects.push_back(std::move(city));

	// load car
	auto car1{ assimpLoad("models/cars/car_1.fbx", true) };
	car1.position = glm::vec3{ 0, -0.7, 0 };
	car1.scale = glm::vec3{ 0.03, 0.03, 0.03 };

	// Move the car into the scene list.
	scene.objects.push_back(std::move(car1));

	return scene;
}

void renderScene(Scene& myScene, ShaderProgram& program) {
	for (auto& o : myScene.objects) {
		program.setUniform("hasNormalMap", o.hasNormalMap);
		program.setUniform("hasSpecularMap", o.hasSpecularMap);
		o.drawObject(program);
	}
}

int main() {

	std::cout << std::filesystem::current_path() << std::endl;

	// Initialize the window and OpenGL.
	sf::ContextSettings settings;
	settings.depthBits = 24; // Request a 24 bits depth buffer
	settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	settings.majorVersion = 3; // You might have to change these on Mac.
	settings.minorVersion = 3;

	sf::Window window{
		sf::VideoMode::getFullscreenModes().at(0), "Modern OpenGL",
		sf::Style::Resize | sf::Style::Close,
		sf::State::Windowed, settings
	};

	gladLoadGL();
	glEnable(GL_DEPTH_TEST);

	// Inintialize scene objects.
	auto myScene{ projectScene() };

	// You can directly access specific objects in the scene using references.
	auto& firstObject{ myScene.objects[0] };
	auto& secondObject{ myScene.objects[1] };

	// Generate and bind a custom framebuffer.
	uint32_t myFbo;
	glGenFramebuffers(1, &myFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, myFbo);
	// Render commands will no longer render to the screen.

	// frame buffer setup: colorBufferID contains an image that stores color
	uint32_t colorBufferId;
	glGenTextures(1, &colorBufferId);
	glBindTexture(GL_TEXTURE_2D, colorBufferId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window.getSize().x, window.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); // allocate GPU memory for 2D image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	// will write to this texture's pixels
	Mesh texturedSquare{ Mesh::square({Texture{ colorBufferId, "baseTexture" }}) };
	

	uint32_t depthBufferId;	// image that stored depth
	glGenTextures(1, &depthBufferId);
	glBindTexture(GL_TEXTURE_2D, depthBufferId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window.getSize().x, window.getSize().y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Bind the two textures as the write-destinations for color and depth.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferId, 0);
	
	
	
	// Generate and bind a depth framebuffer for the "shadow map"
	uint32_t shadowFbo;
	glGenFramebuffers(1, &shadowFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);

	uint32_t shadowDepthID;
	glGenTextures(1, &shadowDepthID);
	glBindTexture(GL_TEXTURE_2D, shadowDepthID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window.getSize().x, window.getSize().y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthID, 0);
	
	// depth only fbo
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	myScene.program.activate(); // Activate the shader program, future draw calls will use this


	// Ready, set, go!
	bool running{ true };
	sf::Clock c;

	ShaderProgram clipSpaceNoLightingProgram = clipSpaceNoLightingProgramShader();
	ShaderProgram depthShaderProgram = depthShader();

	glm::vec3 cameraPos{ glm::vec3{ 0, 50, 70 } };	// camera position
	glm::mat4 camera{ glm::lookAt(cameraPos, glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 1, 0 }) };	// camera matrix
	glm::mat4 perspective{ glm::perspective(glm::radians(45.0f), static_cast<float>(window.getSize().x) / window.getSize().y, 0.1f, 200.0f) };  // perspective matrix
	glm::mat4 orthographic = glm::ortho(
		-80.0f * static_cast<float>(window.getSize().x) / window.getSize().y,
		80.0f * static_cast<float>(window.getSize().x) / window.getSize().y,
		-80.0f,
		80.0f,
		0.1f,
		500.0f
	);

	glm::vec3 target{ 0.0f, 0.0f, 0.0f };   // where camera looks at

	// states
	bool isFlying = false;	
	bool isNight = false;
	bool nWasPressed = false;
	
	auto last{ c.getElapsedTime() };
	while (window.isOpen()) {

		

		// Check for events.
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		// diff measures frame length
		auto now{ c.getElapsedTime() };
		auto diff{ now - last };
		last = now;

		bool nIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::N);

		if (nIsPressed && !nWasPressed) {
			isNight = !isNight;   // TOGGLE
		}

		nWasPressed = nIsPressed;

		float camSpeed = 30.0f * diff.asSeconds();	// speed camera moves
		float camRotSpeed = 1.0f * diff.asSeconds();	// speed camera rotates

		// panning the camera
		// must define a forward direction relative to the ground plane & move where the camera looks at

		// forward = eye (camera position) - at (target point or where camera looks at)
		glm::vec3 lookDir = normalize(target - cameraPos);
		glm::vec3 fwdFlat = glm::normalize(glm::vec3(lookDir.x, 0, lookDir.z)); // x and z are changing, but y-axis is always the same
		glm::vec3 rightFlat = normalize(cross(fwdFlat, glm::vec3{ 0, 1, 0 })); // x and z are changing, but y-axis is always the same

		// camera movement (WASD)
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
			// move the camera a small amount in the direction that points to the right
			cameraPos += fwdFlat * camSpeed;
			target += fwdFlat * camSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
			cameraPos -= rightFlat * camSpeed;
			target -= rightFlat * camSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
			cameraPos -= fwdFlat * camSpeed;
			target -= fwdFlat * camSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
			cameraPos += rightFlat * camSpeed;
			target += rightFlat * camSpeed;
		}

		// Rotating the camera = rotating the vector from target to camera around the target
		float angle = 0.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Q)) {
			angle += camRotSpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)) {
			angle -= camRotSpeed;
		}

		// if camera angle was changed, then change cameraPos
		if (angle != 0.0f) {
			glm::vec3 offset = cameraPos - target;

			// 2. rotate offset around world up (Y)
			glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, glm::vec3{0,1,0});
			offset = glm::vec3(R * glm::vec4(offset, 0.0f));
			cameraPos = target + offset;
		}

		// used to detect movement for bird orientation
		glm::vec3 moveDir(0.0f);
		
		float birdSpeed;

		if (isFlying) { birdSpeed = 30.0f * diff.asSeconds(); }
		else { birdSpeed = 10.0f * diff.asSeconds(); }

		// bird movement relative to the camera: moving (x & z axis only)
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
			moveDir += fwdFlat;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
			moveDir -= fwdFlat;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
			moveDir -= rightFlat;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
			moveDir += rightFlat;
		}

		// if there is movement, move the bird & face correct direction
		if (glm::length(moveDir) > 0.0001f) {
			// normalize so diagonal speed is consistent
			moveDir = glm::normalize(moveDir);

			firstObject.position += moveDir * birdSpeed;

			// make the bird face movement direction
			float yaw = std::atan2(moveDir.x, moveDir.z);
			firstObject.orientation.y = yaw;
		}

		// making the bird fly
		const float groundY = 0.0f; // same as your initial bird position
		float flySpeed = 6.0f * diff.asSeconds();

		// adjust position in y-axis if flying
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space)) {
			firstObject.position.y += flySpeed;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift)) {
			firstObject.position.y -= flySpeed;
		}

		// clamp so bird can never go below ground
		if (firstObject.position.y < groundY) firstObject.position.y = groundY;

		// bird is flying if its off the ground
		if (firstObject.position.y > groundY + 0.001f) { isFlying = true; }
		else { isFlying = false; }
		
		// keep track of time bird flaps wings
		static float flapTime = 0.0f;

		static float spread = glm::radians(80.0f);   // keeps wings away from body 
		static float amp = glm::radians(40.0f);   // bigger flap 

		if (isFlying) {
			flapTime += diff.asSeconds();
			float flap = std::sin(flapTime * 30.0f) * amp;

			// Y-axis flap on the wings (NOT the body)
			firstObject.children[1].orientation.y = -spread + flap;
			firstObject.children[2].orientation.y = spread - flap;
		}
		else {
			flapTime = 0.0f;
			firstObject.children[1].orientation.y = 0.0f;
			firstObject.children[2].orientation.y = 0.0f;
		}


#ifdef LOG_FPS
		// FPS calculation.
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
#endif

		glm::vec3 lightDir = glm::normalize(glm::vec3(-0.35f, -0.75f, -0.55f));
		
		// ---- Lighting uniforms ----
		myScene.program.setUniform("ambientColor", glm::vec3(0.15f, 0.15f, 0.15f));

		// Direction the light points * toward * the scene
		myScene.program.setUniform("directionalLight",
			glm::normalize(glm::vec3(-0.35f, -0.75f, -0.55f)));

		myScene.program.setUniform("directionalColor",
			glm::vec3(1.0f, 0.98f, 0.92f));

		// ---- Material: (ka, kd, ks, shininess) ----
		myScene.program.setUniform("material",
			glm::vec4(
				0.3f,   // ambient strength
				0.8f,   // diffuse strength
				0.4f,   // specular strength
				1.0f   // shininess
			)
		);

		myScene.program.setUniform("attenuationValues",
			glm::vec3(
				1.0f,    // constant
				0.045f,   // linear
				0.0075f   // quadratic
			)
		);

		myScene.program.setUniform("lightPosition",
			glm::vec3(
				-10.0f,
				4.0f,
				-30.0f
			)
		);

		/*
		if (isNight) {
			// ---- Lighting uniforms ----
			myScene.program.setUniform("ambientColor", glm::vec3(0.1f, 0.2f, 0.2f));

			// Direction the light points * toward * the scene
			myScene.program.setUniform("directionalLight",
				glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f)));

			myScene.program.setUniform("directionalColor",
				glm::vec3(1.0f, 1.0f, 1.0f));

			// ---- Material: (ka, kd, ks, shininess) ----
			myScene.program.setUniform("material",
				glm::vec4(
					1.0f,   // ambient strength
					0.8f,   // diffuse strength
					0.4f,   // specular strength
					1.0f   // shininess
				)
			);

			myScene.program.setUniform("attenuationValues",
				glm::vec3(
					1.0f,    // constant
					0.09f,   // linear
					0.032f   // quadratic
				)
			);

			myScene.program.setUniform("lightPosition",
				glm::vec3(
					5.0f,
					0.0f,
					0.0f
				)
			);
		}
		else {
			// ---- Lighting uniforms ----
			myScene.program.setUniform("ambientColor", glm::vec3(0.15f, 0.15f, 0.15f));

			// Direction the light points * toward * the scene
			myScene.program.setUniform("directionalLight",
				glm::normalize(glm::vec3(-0.35f, -0.75f, -0.55f)));

			myScene.program.setUniform("directionalColor",
				glm::vec3(1.0f, 0.98f, 0.92f));

			// ---- Material: (ka, kd, ks, shininess) ----
			myScene.program.setUniform("material",
				glm::vec4(
					0.3f,   // ambient strength
					0.8f,   // diffuse strength
					0.4f,   // specular strength
					1.0f   // shininess
				)
			);

			myScene.program.setUniform("attenuationValues",
				glm::vec3(
					1.0f,    // constant
					0.045f,   // linear
					0.0075f   // quadratic
				)
			);

			myScene.program.setUniform("lightPosition",
				glm::vec3(
					20.0f,
					10.0f,
					5.0f
				)
			);
		}
		*/
		firstObject.hasNormalMap = false;
		secondObject.hasNormalMap = true;
		firstObject.hasSpecularMap = true;
		firstObject.hasSpecularMap = false;

		// PASS 1: Render scene from light POV and make shadow map
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
		glClear(GL_DEPTH_BUFFER_BIT); // Clear the OpenGL "context".

		depthShaderProgram.activate();

		// for directional light, the light source is far away in the opposite direction
		glm::vec3 lightPosForShadow = glm::vec3(0.0f) - lightDir * 200.0f;
		glm::mat4 lightView = glm::lookAt(lightPosForShadow, glm::vec3(0.0f), glm::vec3(0, 1, 0));
		glm::mat4 lightProj = glm::ortho(-120.f, 120.f, -120.f, 120.f, 0.1f, 500.f);
		glm::mat4 lightSpaceMatrix = lightProj * lightView;

		depthShaderProgram.setUniform("lightSpaceMatrix", lightSpaceMatrix);

		renderScene(myScene, depthShaderProgram);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PASS 2: Render scene from top-down camera into minimap FBO
		myScene.program.activate();
		glBindFramebuffer(GL_FRAMEBUFFER, myFbo);
		glViewport(0, 0, window.getSize().x, window.getSize().y);
		glClearColor(0.52f, 0.73f, 0.98f, 1.0f);  // sky blue background color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the OpenGL "context".
		glEnable(GL_DEPTH_TEST);
		
		// set camera position and view mmtrix to be above the bird
		glm::vec3 center = firstObject.position;          // bird position
		glm::vec3 miniPos = center + glm::vec3(0, 200, 0);  // above bird

		glm::mat4 miniView{ glm::lookAt(miniPos, center, glm::vec3{ 0, 0, -1 }) }; // cam/view matrix
		myScene.program.setUniform("view", miniView);
		myScene.program.setUniform("projection", orthographic);
		myScene.program.setUniform("cameraPos", miniPos);
		renderScene(myScene, myScene.program);
		
		// PASS 3: render scene from actual camera's POV with lighting and transforms
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.getSize().x, window.getSize().y);
		glEnable(GL_DEPTH_TEST);
		
		glClearColor(0.52f, 0.73f, 0.98f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myScene.program.activate();

		myScene.program.setUniform("shadowMap", 5);

		// bind shadow map for sampling
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowDepthID);
		myScene.program.setUniform("shadowMap", 5);

		camera = glm::lookAt(cameraPos, target, glm::vec3{ 0, 1, 0 }); // cam/view matrix
		myScene.program.setUniform("view", camera);
		myScene.program.setUniform("projection", perspective);
		myScene.program.setUniform("cameraPos", cameraPos);
		myScene.program.setUniform("lightSpaceMatrix", lightSpaceMatrix);
		renderScene(myScene, myScene.program);
		

		// PASS 4: Draw minimap over
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.getSize().x, window.getSize().y);
		
		clipSpaceNoLightingProgram.activate();	// shader has no lighting and adjusts scale/offset of minimap

		// bind the minimap texture for sampling
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBufferId);
		clipSpaceNoLightingProgram.setUniform("baseTexture", 0);
		texturedSquare.drawMesh(clipSpaceNoLightingProgram); // render minimap to screen
		glEnable(GL_DEPTH_TEST);

		
		

		window.display();

		
	}

	return 0;
}


