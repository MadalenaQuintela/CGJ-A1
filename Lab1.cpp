#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/random.hpp>
#include <glm/gtc/epsilon.hpp>
#include <stdio.h>
#include <iomanip>
#include <numbers>

const float THRESHOLD = (float)1.0e-5;
const float pi = std::numbers::pi;
const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
const glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

glm::vec3 RodriguesRotationFormula(glm::vec3 v, float angle, glm::vec3 a) {
	a = glm::normalize(a);
	return v * cos(angle) + glm::cross(a, v) * sin(angle) + a * (glm::dot(a, v) * (1 - cos(angle)));
}

glm::mat3 CreateRodriguesRotationMatrix(const glm::vec3& rotationAxis, float angle) {
	glm::mat3 I(1.0f);
	glm::vec3 a = glm::normalize(rotationAxis);
	glm::mat3 A(
		0.0f, a.z, -a.y, // column 1
		-a.z, 0.0f, a.x, // column 2
		a.y, -a.x, 0.0f  // column 3
	);
	return I + (std::sin(angle) * A) + ((1.0f - std::cos(angle)) * A * A);
}

float cleanZero(float val) {
	return (std::abs(val) < 1e-5f) ? 0.0f : val;
}

void printResultRodRot(const std::string& axisName, const std::string& rotAxisName, const glm::vec3& actual, const glm::vec3& expected) {
	std::cout << "Rotating " << axisName << " by " << rotAxisName << " (90 deg):\n"
		<< std::fixed << std::setprecision(1)
		<< "Expected Output: " << expected.x << ", " << expected.y << ", " << expected.z << " \n"
		<< "Actual Output: " << cleanZero(actual.x) << ", " << cleanZero(actual.y) << ", " << cleanZero(actual.z) << " \n"
		<< "\n";
}

glm::mat3 generateRandomMatrix() {
	glm::mat3 mat;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mat[i][j] = glm::linearRand(-5.0f, 5.0f);
		}
	}
	return mat;
}

void printMatrix(const glm::mat3& mat) {
	for (int row = 0; row < 3; row++) {
		std::cout << mat[0][row] << ", "
			<< mat[1][row] << ", "
			<< mat[2][row] << '\n';
	}
}

bool matricesEqual(const glm::mat3& a, const glm::mat3& b) {
	for (int i = 0; i < 3; i++) {
		glm::vec3 tolerance = THRESHOLD * glm::max(
			glm::vec3(1.0f), 
			glm::max(glm::abs(a[i]), glm::abs(b[i]))
		); // check how much they can differ until we consider them actually unequal
		if (!glm::all(glm::lessThanEqual(glm::abs(a[i] - b[i]), tolerance))) {
			return false;
		}
	}
	return true;
}

void testTripleProductProp() {
	for (int test = 1; test <= 5; test++) {
		glm::vec3 i = glm::linearRand(glm::vec3(1.0f), glm::vec3(5.0f));
		glm::vec3 j = glm::linearRand(glm::vec3(1.0f), glm::vec3(5.0f));
		glm::vec3 k = glm::linearRand(glm::vec3(1.0f), glm::vec3(5.0f));

		glm::vec3 first = glm::cross(i, glm::cross(j, k));
		glm::vec3 second = j * glm::dot(i, k) - k * glm::dot(i, j);

		// bool equal = glm::all(glm::epsilonEqual(first, second, THRESHOLD));
		// floating point errors would make some random vectors to fail with the simple test above
		glm::vec3 tolerance = THRESHOLD * glm::max(glm::vec3(1.0f), glm::max(glm::abs(first), glm::abs(second)));
		bool equal = glm::all(glm::lessThanEqual(glm::abs(first - second), tolerance));

		std::cout << "Test " << test << ":\n";
		std::cout << "i: " << i.x << ", " << i.y << ", " << i.z << "\n";
		std::cout << "j: " << j.x << ", " << j.y << ", " << j.z << "\n";
		std::cout << "k: " << k.x << ", " << k.y << ", " << k.z << "\n";
		std::cout << "Result: " << (equal ? "PASS" : "FAIL") << "\n\n";
	}
}

void testCameraFrame(glm::vec3 view, glm::vec3 up) {
	glm::vec3 v = view / glm::length(view);
	glm::vec3 cross = glm::cross(up, v);
	if (glm::length(cross) < THRESHOLD) {
		std::cout << "Cannot create coordinate frame, view and up are parallel!\n";
		// could return here but output is still interesting
	}

	glm::vec3 w = cross / glm::length(cross);
	glm::vec3 u = glm::cross(v, w);
	std::cout << "v: " << v.x << ", " << v.y << ", " << v.z
		<< " u: " << u.x << ", " << u.y << ", " << u.z
		<< " w: " << w.x << ", " << w.y << ", " << w.z << std::endl;
}

void testCoordinateFrameForCamera() {
	std::cout << "Test 1:\n";
	testCameraFrame(glm::vec3(2.0f, 3.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	std::cout << "Test 2:\n";
	testCameraFrame(glm::vec3(3.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	std::cout << "Test 3:\n";
	testCameraFrame(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void testRodriguesRotation() {
	glm::vec3 vec1 = glm::vec3(1.0f, 1.0f, 1.0f);
	float angle = pi / 2.0f;
	std::cout << "Input 1: " << vec1.x << ", " << vec1.y << ", " << vec1.z << ", angle " << angle << " rad, z axis" << std::endl;
	glm::vec3 res1 = RodriguesRotationFormula(vec1, angle, zAxis);
	glm::vec3 expected1(-1.0f, 1.0f, 1.0f);
	std::cout << "Expected: " << expected1.x << ", " << expected1.y << ", " << expected1.z << std::endl;
	std::cout << "Output 1: vector: " << res1.x << ", " << res1.y << ", " << res1.z << std::endl;
	std::cout << "Result: " << (glm::all(glm::epsilonEqual(res1, expected1, THRESHOLD)) ? "PASS" : "FAIL") << "\n\n";

	vec1 = glm::vec3(4.0f, 1.0f, 2.0f);
	angle = 30 * pi / 180.0f;
	std::cout << "Input 2: " << vec1.x << ", " << vec1.y << ", " << vec1.z << ", angle " << angle << " rad, y axis" << std::endl;
	glm::vec3 res2 = RodriguesRotationFormula(vec1, angle, yAxis);
	glm::vec3 expected2(4.4641016f, 1.0f, -0.2679492f);
	std::cout << "Expected: " << expected2.x << ", " << expected2.y << ", " << expected2.z << std::endl;
	std::cout << "Output 2: vector: " << res2.x << ", " << res2.y << ", " << res2.z << std::endl;
	std::cout << "Result: " << (glm::all(glm::epsilonEqual(res2, expected2, THRESHOLD)) ? "PASS" : "FAIL") << "\n\n";

	vec1 = glm::vec3(5.0f, 3.0f, 1.0f);
	angle = 60 * pi / 180.0f;
	std::cout << "Input 3: " << vec1.x << ", " << vec1.y << ", " << vec1.z << ", angle " << angle << " rad, x axis" << std::endl;
	glm::vec3 res3 = RodriguesRotationFormula(vec1, angle, xAxis);
	glm::vec3 expected3(5.0f, 0.6339746f, 3.0980762f);
	std::cout << "Expected: " << expected3.x << ", " << expected3.y << ", " << expected3.z << std::endl;
	std::cout << "Output 3: vector: " << res3.x << ", " << res3.y << ", " << res3.z << std::endl;
	std::cout << "Result: " << (glm::all(glm::epsilonEqual(res3, expected3, THRESHOLD)) ? "PASS" : "FAIL") << "\n\n";
}

void testRodriguesRotationMatrix() {
	float angle90 = pi / 2.0f;
	glm::mat3 rotX = CreateRodriguesRotationMatrix(xAxis, angle90);
	glm::mat3 rotY = CreateRodriguesRotationMatrix(yAxis, angle90);
	glm::mat3 rotZ = CreateRodriguesRotationMatrix(zAxis, angle90);

	std::cout << "Rotating x axis: ";
	printResultRodRot("X-Axis", "X-Axis", rotX * xAxis, glm::vec3(1.0f, 0.0f, 0.0f));
	printResultRodRot("X-Axis", "Y-Axis", rotY * xAxis, glm::vec3(0.0f, 0.0f, -1.0f));
	printResultRodRot("X-Axis", "Z-Axis", rotZ * xAxis, glm::vec3(0.0f, 1.0f, 0.0f));

	std::cout << "Rotating y axis: ";
	printResultRodRot("Y-Axis", "X-Axis", rotX * yAxis, glm::vec3(0.0f, 0.0f, 1.0f));
	printResultRodRot("Y-Axis", "Y-Axis", rotY * yAxis, glm::vec3(0.0f, 1.0f, 0.0f));
	printResultRodRot("Y-Axis", "Z-Axis", rotZ * yAxis, glm::vec3(-1.0f, 0.0f, 0.0f));

	std::cout << "Rotating z axis: ";
	printResultRodRot("Z-Axis", "X-Axis", rotX * zAxis, glm::vec3(0.0f, -1.0f, 0.0f));
	printResultRodRot("Z-Axis", "Y-Axis", rotY * zAxis, glm::vec3(1.0f, 0.0f, 0.0f));
	printResultRodRot("Z-Axis", "Z-Axis", rotZ * zAxis, glm::vec3(0.0f, 0.0f, 1.0f));
}

void testMatrixTranspose(int experiments) {
	std::cout << "Testing " << experiments << " experiments\n";
	for (int i = 0; i < experiments; i++) {
		std::cout << "Experiment " << i + 1 << ":\n";

		glm::mat3 A = generateRandomMatrix();
		glm::mat3 B = generateRandomMatrix();
		glm::mat3 first = glm::transpose(A * B);
		glm::mat3 second = glm::transpose(B) * glm::transpose(A);

		std::cout << "A:\n";
		printMatrix(A);
		std::cout << "B:\n";
		printMatrix(B);

		std::cout << "\n(AB)^T:\n";
		printMatrix(first);
		std::cout << "\nB^T * A^T:\n";
		printMatrix(second);

		bool equal = matricesEqual(first, second);
		std::cout << "Result: " << (equal ? "PASS" : "FAIL") << "\n\n";
	}
}

// Floating point errors get really amplified whith low determinants so the pass print isnt always correct since the threshold is too strict
// Therefore generate random matricies with determinant greater than 1
glm::mat3 generateRandomInvertibleMatrix() {
	glm::mat3 mat;
	do {
		mat = generateRandomMatrix();
	} while (std::abs(glm::determinant(mat)) < 1.0f);
	return mat;
}

void testMatrixInverse(int experiments) {
	std::cout << "Testing " << experiments << " experiments\n";
	for (int i = 0; i < experiments; i++) {
		std::cout << "Experiment " << i + 1 << ":\n";

		glm::dmat3 A = generateRandomInvertibleMatrix();
		glm::dmat3 B = generateRandomInvertibleMatrix();
		glm::mat3 first = glm::inverse(A * B);
		glm::mat3 second = glm::inverse(B) * glm::inverse(A);

		std::cout << "A:\n";
		printMatrix(A);
		std::cout << "B:\n";
		printMatrix(B);

		std::cout << "\n(AB)^{-1}:\n";
		printMatrix(first);
		std::cout << "\nB^{-1} * A^{-1}:\n";
		printMatrix(second);

		bool equal = matricesEqual(first, second);
		std::cout << "Result: " << (equal ? "PASS" : "FAIL") << "\n\n";
	}
}

int main() {
	// Exercise 1
	std::cout << "Exercise 1: Testing Triple Product Property" << std::endl;
	testTripleProductProp();

	// Exercise 2
	std::cout << " \n \nExercise 2: Testing Coordinate Frame for Camera" << std::endl;
	testCoordinateFrameForCamera();

	// Exercise 3
	std::cout << " \n \nExercise 3: Testing Rodrigues Rotation Formula" << std::endl;
	testRodriguesRotation();

	// Exercise 4
	std::cout << " \n \nExercise 4: Testing Rodrigues Rotation Formula (Matrix Form)" << std::endl;
	testRodriguesRotationMatrix();

	// Exercise 5
	std::cout << " \n \nExercise 5: Testing transpose property" << std::endl;
	testMatrixTranspose(5);

	// Exercise 6
	std::cout << " \n \nExercise 6: Testing inverse property" << std::endl;
	testMatrixInverse(5);

	return 0;
}
