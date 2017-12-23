
/* RBJ filter based on M4W code */

class filter2p {
private:
	double *filtercoefsTab;

	int filtertype;
	double *coefsTabOffs;
	double d0, d1;
	double d0r, d1r;

	double a1, a2, b0, b1, b2;
	void Compute(double f, double q, int t);
	void ComputeCoefs( double *coefs, int freq, int r, int t, int SampleRate);

public:

	filter2p() { }
	void init(int SampleRate, double **coefstab);

	void reset() {
		d0 = d1 = 0;
		d0r = d1r = 0;
	}
	
	void settype(unsigned char type);
	void dofilter(float *data, int length, float cutoffvalue, float resonancevalue);
	void dofilter_s(float *data, int length, float cutoffvalue, float resonancevalue);
};
