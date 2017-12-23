#if defined __cplusplus
extern "C" {
#endif

extern unsigned char mem[];

void initcpu(unsigned short newpc, unsigned char newa, unsigned char newx, unsigned char newy);
int runcpu(void);

#if defined __cplusplus
}
#endif