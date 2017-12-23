void CopyM2S(float *pout, float *pin, int numsamples, float amp);

void CopyStereoToMono(float *pout, float *pin, int numsamples, float amp)
{
	do
	{
		*pout++ = (pin[0] + pin[1]) * amp;
		pin += 2;
	} while(--numsamples);
}
/*
void Amp(float *pout, int numsamples, float amp) {
	for (int i=0; i<numsamples; i++) {
		pout[i]*=amp;
	}
}*/

void s2i(float *i, float **s, int numsamples) {
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0]++;
			*i++ = 0;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = 0;
			*i++ = *p[1]++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0]++;
			*i++ = *p[1]++;
		}
	}
}

void s2i_amp(float *i, float **s, int channels, int numsamples, float amp) {
	for (int k = 0; k < numsamples; k++) {
		float v1 = 0, v2 = 0;
		for (int j = 0; j < channels; j++)
			if (s[j] != 0)
				if ((j&1) == 0)
					v1 += s[j][k];
				else
					v2 += s[j][k];
		*i++ = v1 * amp;
		*i++ = v2 * amp;
	}
}

void s2i_amp(float *i, float **s, int numsamples, float amp) {
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0] * amp;
			*i++ = 0;
			p[0]++;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = 0;
			*i++ = *p[1] * amp;
			p[1]++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0] * amp;
			*i++ = *p[1] * amp;
			p[0]++;
			p[1]++;
		}
	}
}

void i2s(float **s, float *i, int numsamples) {
	if (!numsamples)
		return;
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--) {
			*p[0]++ = *i++;
			i++;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--) {
			i++;
			*p[1]++ = *i++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--) {
			*p[0]++ = *i++;
			*p[1]++ = *i++;
		}
	}
}

void i2s_amp(float **s, float *i, int numsamples, float amp) {
	if (!numsamples)
		return;
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--) {
			*p[0]++ = *i * amp;
			i++;
			i++;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--) {
			i++;
			*p[1]++ = *i * amp;
			i++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--) {
			*p[0]++ = *i * amp;
			i++;
			*p[1]++ = *i * amp;
			i++;
		}
	}
}
