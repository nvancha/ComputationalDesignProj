#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include <FL/gl.h>
#include <GL/glut.h>
#include "modelerglobals.h"
#include <cstdlib>
#include <ctime>

const int display_interval = 50;

class MyModel : public ModelerView
{
public:
	MyModel(int x, int y, int w, int h, char *label)
		: ModelerView(x, y, w, h, label) { }

	virtual void draw();
	void triggerRedraw() { damage(1); }
};

ModelerView* createSampleModel(int x, int y, int w, int h, char *label)
{
	return new MyModel(x, y, w, h, label);
}

bool isWithinRoomBoundaries(float x, float z, float roomSize, float margin) {
	return x >= margin && x <= roomSize - margin && z >= margin && z <= roomSize - margin;
}

void optimizeFurnitureLocations(MyModel* view) {
	// Initialization
	const int num_of_iterations = 1000;
	float t = 1000.0f;

	// Define helper functions for cost functions and MoveT/MoveR
	auto pairwiseDistanceCostFunction = [](float d, float target_d) {
		return pow(d - target_d, 2);
	};

	auto pairwiseAngleCostFunction = [](float angle, float target_angle) {
		float angle_diff = fmod(abs(angle - target_angle), 360);
		return min(angle_diff, 360 - angle_diff);
	};

	auto moveT = [](float &x, float &z, float angle, float distance) {
		x += distance * cos(angle * M_PI / 180);
		z += distance * sin(angle * M_PI / 180);
	};

	auto moveR = [](float &angle, float theta) {
		angle = fmod(angle + theta, 360);
	};

	// Main optimization loop
	for (int i = 1; i <= num_of_iterations; i++) {
		// Update temperature schedule
		if (i <= 400)
			t = 1000;
		else if (i > 400 && i <= 800)
			t = 100;
		else if (i > 800 && i <= 1000)
			t = 10;

		// Compute current cost
		float cur_couch_angle = GETVAL(TARGET_ANGLE_COUCH_TO_TV);
		float cur_tv_angle = GETVAL(TARGET_ANGLE_TV_TO_COUCH);
		float cur_couch_x = GETVAL(COUCH_XPOS);
		float cur_couch_z = GETVAL(COUCH_ZPOS);
		float cur_tv_x = GETVAL(TV_XPOS);
		float cur_tv_z = GETVAL(TV_ZPOS);

		float d = sqrt(pow(cur_couch_x - cur_tv_x, 2) + pow(cur_couch_z - cur_tv_z, 2));
		float cur_cost = GETVAL(WEIGHT_PAIRWISE_DISTANCE) * pairwiseDistanceCostFunction(d, GETVAL(TARGET_PAIRWISE_DISTANCE))
			+ GETVAL(WEIGHT_PAIRWISE_ANGLE) * pairwiseAngleCostFunction(cur_couch_angle, cur_tv_angle);

		// Randomly pick a point (furniture) and a function (MoveT or MoveR)
		int random_furniture = rand() % 2;
		int random_function = rand() % 2;
		float random_distance = ((rand() % 100) / 100.0f) * 2 - 1; // Random value between -1 and 1
		float random_angle = ((rand() % 100) / 100.0f) * 10 - 5; // Random value between -5 and 5

		// Move the selected furniture (couch or TV) using the selected function (MoveT or MoveR)
		if (random_furniture == 0) { // Couch
			if (random_function == 0) {
				moveT(cur_couch_x, cur_couch_z, cur_couch_angle, random_distance);
			}
			else {
				moveR(cur_couch_angle, random_angle);
			}
		}
		else { // TV
			if (random_function == 0) {
				moveT(cur_tv_x, cur_tv_z, cur_tv_angle, random_distance);
			}
			else {
				moveR(cur_tv_angle, random_angle);
			}
		}

		// Compute new cost
		d = sqrt(pow(cur_couch_x - cur_tv_x, 2) + pow(cur_couch_z - cur_tv_z, 2));
		float new_cost = GETVAL(WEIGHT_PAIRWISE_DISTANCE) * pairwiseDistanceCostFunction(d, GETVAL(TARGET_PAIRWISE_DISTANCE))
			+ GETVAL(WEIGHT_PAIRWISE_ANGLE) * pairwiseAngleCostFunction(cur_couch_angle, cur_tv_angle);

		// Metropolis-Hastings acceptance criterion
		float p = exp((cur_cost - new_cost) / t);
		float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

		if ((new_cost < cur_cost || r < p) && isWithinRoomBoundaries(cur_couch_x, cur_couch_z, GETVAL(FLOOR_SIZE), 2) && isWithinRoomBoundaries(cur_tv_x, cur_tv_z, GETVAL(FLOOR_SIZE), 2)) {
			// Accept proposed layout
			ModelerApplication::Instance()->SetControlValue(COUCH_XPOS, cur_couch_x);
			ModelerApplication::Instance()->SetControlValue(COUCH_ZPOS, cur_couch_z);
			ModelerApplication::Instance()->SetControlValue(TV_XPOS, cur_tv_x);
			ModelerApplication::Instance()->SetControlValue(TV_ZPOS, cur_tv_z);
			ModelerApplication::Instance()->SetControlValue(TARGET_ANGLE_COUCH_TO_TV, cur_couch_angle);
			ModelerApplication::Instance()->SetControlValue(TARGET_ANGLE_TV_TO_COUCH, cur_tv_angle);
		}

		// Show intermediate results and optimization info
		if (i % display_interval == 0) {
			if (GETVAL(SHOW_INTERMEDIATE_RESULTS)) {
				if (GETVAL(SHOW_INTERMEDIATE_RESULTS)) {
					view->triggerRedraw();
					Fl::check(); // Process any pending events
				}

			}

			printf("Iteration: %d\n", i);
			printf("Total Cost: %f\n", cur_cost);
			printf("Pairwise Distance Cost: %f\n", GETVAL(WEIGHT_PAIRWISE_DISTANCE) * pairwiseDistanceCostFunction(d, GETVAL(TARGET_PAIRWISE_DISTANCE)));
			printf("Pairwise Angle Cost: %f\n", GETVAL(WEIGHT_PAIRWISE_ANGLE) * pairwiseAngleCostFunction(cur_couch_angle, cur_tv_angle));
			printf("\n");
		}
	}
}

void resetFurnitureLocations() {
	srand(time(0));
	float roomSize = GETVAL(FLOOR_SIZE);
	float margin = 2.0; // Keep furniture away from the walls

	float random_couch_x = margin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (roomSize - 2 * margin)));
	float random_couch_z = margin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (roomSize - 2 * margin)));

	float random_tv_x = margin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (roomSize - 2 * margin)));
	float random_tv_z = margin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (roomSize - 2 * margin)));

	ModelerApplication::Instance()->SetControlValue(COUCH_XPOS, random_couch_x);
	ModelerApplication::Instance()->SetControlValue(COUCH_ZPOS, random_couch_z);

	ModelerApplication::Instance()->SetControlValue(TV_XPOS, random_tv_x);
	ModelerApplication::Instance()->SetControlValue(TV_ZPOS, random_tv_z);
}

void MyModel::draw()
{
	ModelerView::draw();

	if (GETVAL(RESET_FURNITURE)) {
		resetFurnitureLocations();
		ModelerApplication::Instance()->SetControlValue(RESET_FURNITURE, 0);
	}

	if (GETVAL(OPTIMIZE_FURNITURE)) {
		optimizeFurnitureLocations(this);
		ModelerApplication::Instance()->SetControlValue(OPTIMIZE_FURNITURE, 0);
	}


	// Draw the floor
	setAmbientColor(.1f, .1f, .1f);
	setDiffuseColor(0.0f, 0.0f, 0.5f);
	glPushMatrix();
	glTranslated(GETVAL(FLOOR_SIZE) / 2, 0, GETVAL(FLOOR_SIZE) / 2);
	drawBox(GETVAL(FLOOR_SIZE), 0.1f, GETVAL(FLOOR_SIZE));
	glTranslated(-GETVAL(FLOOR_SIZE) / 2, 0, -GETVAL(FLOOR_SIZE) / 2);
	glPopMatrix();

	// Compute the angle between the couch and the TV
	float dx = GETVAL(TV_XPOS) - GETVAL(COUCH_XPOS);
	float dz = GETVAL(TV_ZPOS) - GETVAL(COUCH_ZPOS);
	float angle = atan2(dz, dx) * 180 / M_PI;

	// Draw the couch
	glPushMatrix();
	setAmbientColor(COLOR_ORANGE);
	setDiffuseColor(COLOR_ORANGE);
	glTranslated(GETVAL(COUCH_XPOS), 0.5f, GETVAL(COUCH_ZPOS));
	glRotated(GETVAL(TARGET_ANGLE_COUCH_TO_TV), 0.0, 1.0, 0.0);
	drawBox(2.0f, 1, 1.0f);

	// Draw the Couch front vector
	setDiffuseColor(COLOR_RED);
	glPushMatrix();
	drawVector(0.0f, 0, 0.0f, 3);
	glPopMatrix();

	// Draw the TV
	glPushMatrix();
	setAmbientColor(COLOR_GREEN);
	setDiffuseColor(COLOR_GREEN);
	glTranslated(GETVAL(TV_XPOS), 0.5f, GETVAL(TV_ZPOS));
	glRotated(GETVAL(TARGET_ANGLE_TV_TO_COUCH), 0.0, 1.0, 0.0);
	drawBox(1.0f, 0.75f, 0.1f);

	// Draw the TV front vector
	setDiffuseColor(COLOR_RED);
	glPushMatrix();
	drawVector(0.0f, 0, 0.0f, 3);
	glPopMatrix();

	// Debug text
	drawStringOnScreen(ModelerApplication::Instance()->outputString);
}

int main()
{
	ModelerControl controls[NUMCONTROLS];
	controls[MAXLOOPCOUNTER] = ModelerControl("Max Loop Counter", 100, 5000, 1.00f, 2000);
	controls[COUCH_XPOS] = ModelerControl("Couch X Position", 0, 30, 0.1f, 2);
	controls[COUCH_ZPOS] = ModelerControl("Couch Z Position", 0, 30, 0.1f, 2);
	controls[TV_XPOS] = ModelerControl("TV X Position", 0, 30, 0.1f, 8);
	controls[TV_ZPOS] = ModelerControl("TV Z Position", 0, 30, 0.1f, 8);

	controls[FLOOR_SIZE] = ModelerControl("Size Of Floor", 10, 30, 0.1f, 20);

	// Add a control widget for target pairwise distance between the couch and the TV
	controls[TARGET_PAIRWISE_DISTANCE] = ModelerControl("Target Pairwise Distance", 0, 10, 0.1f, 2);

	// Add control widgets for angle targets
	controls[TARGET_ANGLE_TV_TO_COUCH] = ModelerControl("Target Angle: TV to Couch", 0, 360, 1, 0);
	controls[TARGET_ANGLE_COUCH_TO_TV] = ModelerControl("Target Angle: Couch to TV", 0, 360, 1, 180);

	// Add control widgets for weight of pairwise distance and pairwise angle cost terms
	controls[WEIGHT_PAIRWISE_DISTANCE] = ModelerControl("Weight Pairwise Distance", 0, 1.0, 0.1f, 0.5);
	controls[WEIGHT_PAIRWISE_ANGLE] = ModelerControl("Weight Pairwise Angle", 0, 1.0, 0.1f, 0.5);

	// Add a control widget for temperature
	controls[TEMPERATURE] = ModelerControl("Temperature", 0, 1000, 1, 1000);

	controls[RESET_FURNITURE] = ModelerControl("Reset Furniture Locations", 0, 1, 1, 0);// Add a control widget to reset the furniture

	controls[OPTIMIZE_FURNITURE] = ModelerControl("Optimize Furniture Locations", 0, 1, 1, 0);// Add a control widget to optimize the furniture

	controls[SHOW_INTERMEDIATE_RESULTS] = ModelerControl("Show Intermediate Results", 0, 1, 1, 0); // Add a widget to visualize the optimization

	ModelerApplication::Instance()->Init(&createSampleModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
