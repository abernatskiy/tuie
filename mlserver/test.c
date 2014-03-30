#include <stdio.h>
#include <stdlib.h>

int main() {

	printf("Here comes the test...\n");
	int i;

	FILE* training = fopen("input", "w");
	fprintf(training, "train\n");
	fprintf(training, "%le %le %le\n", 0.0, 0.0, 0.0);
	fprintf(training, "%le %le %le\n", 0.5, 0.5, 0.0);
	fprintf(training, "%le %le %le\n", 1.0, 1.1, 1.0);
	fprintf(training, "%le %le %le\n", 1.5, 1.2, 1.0);
	fprintf(training, "%le %le %le\n", 2.0, 2.5, 1.0);
	fclose(training);

	printf("Training data send to the server\n");

	FILE* output = fopen("output", "r");
	char str[100];
	for(i=0; i<1; i++) {
		fscanf(output, "%s\n", str);
		printf("Response: %s\n", str);
	}
	fclose(output);

	FILE* pred = fopen("input", "w");
	fprintf(pred, "predict\n");
	for(i=0; i<10; i++)
		fprintf(pred, "%le %le\n", 0.5*((double) i), 0.5*((double) i));
	fclose(pred);

	printf("Points for prediction are sent to the server\n");

	output = fopen("output", "r");
	double p;
	for(i=0; i<10; i++) {
		fscanf(output, "%lf\n", &p);
		printf("%le\n", p);
	}
	fclose(output);

	return 0;
}
