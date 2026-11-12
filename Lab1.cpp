#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/epsilon.hpp>
#include <stdio.h>
#include <iomanip>

const float THRESHOLD = (float)1.0e-5;

glm::vec3 RodriguesRotationFormula(glm::vec3 v, float angle, glm::vec3 a) {

	return v * cos(angle) + glm::cross(a, v) * sin(angle) + a * (glm::dot(a, v) * (1 - cos(angle)));
}

glm::mat3 RodriguesRotationMatrix(const glm::vec3& rotationAxis, float angle) {
	glm::vec3 k = glm::normalize(rotationAxis);

	glm::mat3 I(1.0f);

	glm::mat3 A(
		0.0f, k.z, -k.y,  
		-k.z, 0.0f, k.x,  
		k.y, -k.x, 0.0f   
	);

	glm::mat3 A2 = A * A;

	glm::mat3 R = I + (std::sin(angle) * A) + ((1.0f - std::cos(angle)) * A2);

	return R;
}

float cleanZero(float val) {
	return (std::abs(val) < 1e-5f) ? 0.0f : val;
}

void printResultRodRot(const std::string& axisName, const std::string& rotAxisName,
	const glm::vec3& expected, const glm::vec3& actual) {
	std::cout << "Rotate " << axisName << " by " << rotAxisName << " (90 deg):\n"
		<< std::fixed << std::setprecision(1)
		<< "  Expected Output: [" << expected.x << ", " << expected.y << ", " << expected.z << "]\n"
		<< "  Actual Output:   [" << cleanZero(actual.x) << ", " << cleanZero(actual.y) << ", " << cleanZero(actual.z) << "]\n\n";
}

int main(){
	std::cout << "Exercise 1: " << std::endl;
	std::cout << "Enter the x, y, z coordinates of the first vector (values between 1 and 5): ";
	float x1, y1, z1;
	std::cin >> x1 >> y1 >> z1;

	std::cout << "Enter the x, y, z coordinates of the second vector (values between 1 and 5): ";
	float x2, y2, z2;
	std::cin >> x2 >> y2 >> z2;

	std::cout << "Enter the x, y, z coordinates of the third vector (values between 1 and 5): ";
	float x3, y3, z3;
	std::cin >> x3 >> y3 >> z3;

	glm::vec3 i(x1, y1, z1);
	glm::vec3 j(x2, y2, z2);
	glm::vec3 k(x3, y3, z3);

	glm::vec3 first = glm::cross(i, glm::cross(j, k));
	glm::vec3 second = j * glm::dot(i, k) - k * glm::dot(i, j);


	if (glm::all(glm::epsilonEqual(first, second, THRESHOLD))) {
		std::cout << "It is true that i x (j x k) = j(i . k) - k(i . j)" << std::endl;
	};

	std::cout << "Exercise 2: " << std::endl;
	std::cout << "Test 1:" << std::endl;

	glm::vec3 view(2.0f, 3.0f, 5.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	glm::vec3 v;
	glm::vec3 u;
	glm::vec3 w;

	v = view / glm::length(view);
	w = glm::cross(up, v) / glm::length(glm::cross(up, v));
	u = glm::cross(v, w);
	std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << " u: " << u.x << ", " << u.y << ", " << u.z << " w: " << w.x << ", " << w.y << ", " << w.z << std::endl;

	std::cout << "Test 2:" << std::endl;

	view = glm::vec3 (3.0f, 2.0f, 5.0f);
	up =  glm::vec3 (0.0f, 1.0f, 0.0f);

	v = view / glm::length(view);
	w = glm::cross(up, v) / glm::length(glm::cross(up, v));
	u = glm::cross(v, w);
	std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << " u: " << u.x << ", " << u.y << ", " << u.z << " w: " << w.x << ", " << w.y << ", " << w.z << std::endl;

	std::cout << "Test 3:" << std::endl;

	view = glm::vec3 (0.0f, 1.0f, 0.0f);
	up = glm::vec3 (0.0f, 1.0f, 0.0f);

	v = view / glm::length(view);
	w = glm::cross(up, v) / glm::length(glm::cross(up, v));
	u = glm::cross(v, w);
	std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << " u: " << u.x << ", " << u.y << ", " << u.z << " w: " << w.x << ", " << w.y << ", " << w.z << std::endl;


	std::cout << "Exercise 3: " << std::endl;

	std::cout << "Input 1: vector (1, 2, 3), angle 0.524 rad, z axis" << std::endl;
	glm::vec3 res1 = RodriguesRotationFormula(glm::vec3(1.0f, 1.0f, 0.0f), 3.1415/2, glm::vec3(0.0f, 0.0f, 1.0f));
	std::cout << "Output 1: vector: " << res1.x << ", " << res1.y << ", " << res1.z << std::endl;

	std::cout << "Input 2: vector (4, 1, 2), angle 0.524 rad, y axis" << std::endl;
	glm::vec3 res2 = RodriguesRotationFormula(glm::vec3(4.0f, 1.0f, 2.0f), 0.524, glm::vec3(0.0f, 1.0f, 0.0f));
	std::cout << "Output 2: vector: " << res2.x << ", " << res2.y << ", " << res2.z << std::endl;

	std::cout << "Input 3: vector (5, 3, 1), angle 1.047 rad, x axis" << std::endl;
	glm::vec3 res3 = RodriguesRotationFormula(glm::vec3(5.0f, 3.0f, 1.0f), 1.047, glm::vec3(1.0f, 0.0f, 0.0f));
	std::cout << "Output 3: vector: " << res3.x << ", " << res3.y << ", " << res3.z << std::endl;

	std::cout << "Exercise 4: " << std::endl;

	float angle90 = 3.14159f / 2.0f; // 90 degrees in radians

	// Define the primary axes
	glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
	glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
	glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

	// Get rotation matrices for 90 degrees around X, Y, and Z
	glm::mat3 rotX = RodriguesRotationMatrix(xAxis, angle90);
	glm::mat3 rotY = RodriguesRotationMatrix(yAxis, angle90);
	glm::mat3 rotZ = RodriguesRotationMatrix(zAxis, angle90);

	// Apply rotations and print both expected and actual results
	std::cout << "--- Rotating X-Axis [1.0, 0.0, 0.0] ---\n";
	printResultRodRot("X-Axis", "X-Axis", glm::vec3(1.0f, 0.0f, 0.0f), rotX * xAxis);
	printResultRodRot("X-Axis", "Y-Axis", glm::vec3(0.0f, 0.0f, -1.0f), rotY * xAxis);
	printResultRodRot("X-Axis", "Z-Axis", glm::vec3(0.0f, 1.0f, 0.0f), rotZ * xAxis);

	std::cout << "--- Rotating Y-Axis [0.0, 1.0, 0.0] ---\n";
	printResultRodRot("Y-Axis", "X-Axis", glm::vec3(0.0f, 0.0f, 1.0f), rotX * yAxis);
	printResultRodRot("Y-Axis", "Y-Axis", glm::vec3(0.0f, 1.0f, 0.0f), rotY * yAxis);
	printResultRodRot("Y-Axis", "Z-Axis", glm::vec3(-1.0f, 0.0f, 0.0f), rotZ * yAxis);

	std::cout << "--- Rotating Z-Axis [0.0, 0.0, 1.0] ---\n";
	printResultRodRot("Z-Axis", "X-Axis", glm::vec3(0.0f, -1.0f, 0.0f), rotX * zAxis);
	printResultRodRot("Z-Axis", "Y-Axis", glm::vec3(1.0f, 0.0f, 0.0f), rotY * zAxis);
	printResultRodRot("Z-Axis", "Z-Axis", glm::vec3(0.0f, 0.0f, 1.0f), rotZ * zAxis);

	return 0;
}