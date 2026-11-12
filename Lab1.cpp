#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/epsilon.hpp>
#include <stdio.h>

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

}