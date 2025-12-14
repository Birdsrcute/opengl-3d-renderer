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

	// Activate the shader program.
	myScene.program.activate();

	// ---- Lighting uniforms ----
	myScene.program.setUniform("ambientColor", glm::vec3(0.2f, 0.2f, 0.2f));

	// Direction the light points * toward * the scene
	myScene.program.setUniform("directionalLight",
		glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f)));

	myScene.program.setUniform("directionalColor",
		glm::vec3(1.0f, 1.0f, 1.0f));

	// ---- Material: (ka, kd, ks, shininess) ----
	myScene.program.setUniform("material",
		glm::vec4(
			0.2f,   // ambient strength
			0.8f,   // diffuse strength
			0.4f,   // specular strength
			32.0f   // shininess
		)
	);

	myScene.program.setUniform("attenuationValues",
		glm::vec3(
			1.0f,    // constant
			0.09f,   // linear
			0.032f   // quadratic
		)
	);

	// Ready, set, go!
	bool running{ true };
	sf::Clock c;

	// Set background color
	glClearColor(0.52f, 0.73f, 0.98f, 1.0f);  // sky blue

	glm::vec3 cameraPos{ glm::vec3{ 0, 50, 70 } };	// camera position
	glm::mat4 camera{ glm::lookAt(cameraPos, glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 1, 0 }) };	// camera matrix
	glm::mat4 perspective{ glm::perspective(glm::radians(45.0f), static_cast<float>(window.getSize().x) / window.getSize().y, 0.1f, 200.0f) };  // perspective matrix
	
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };   // where camera looks at

	bool isFlying = false;	// state 
	
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

		glm::mat4 camera{ glm::lookAt(cameraPos, target, glm::vec3{ 0, 1, 0 }) }; // cam

		myScene.program.setUniform("view", camera);
		myScene.program.setUniform("projection", perspective);
		myScene.program.setUniform("cameraPos", cameraPos);
		
		// Clear the OpenGL "context".
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render the scene objects.
		for (auto& o : myScene.objects) {
			o.drawObject(myScene.program);
		}
		window.display();
	}

	return 0;
}


