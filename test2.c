int main(void) {

	int i, A[310], C[60], D[60];

	for(i = 2; i < 50; i++) {
		A[3*i-5] = C[i];
		D[i] = A[6*i+1];
	}
	
	return 0;
}

