#include<stdio.h>
#include<omp.h>
#include<stdlib.h>
#define NUM_THREADS 2
#define NUM 1000000
#define PART 500000
#define PART1 250000
int compare_ints(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}
int search(int *a,int b,int m,int n){
	int t;
	t = (m+n)/2;
	if(m<=n){
		if(b == a[t])
			return t;
		else if(b > a[t])
			return search(a,b,t+1,n);
		else
			return search(a,b,m,t-1);
	}
	else
		return n;

}
void merge(int *B, int *C, int*A, int b1, int b2, int c1, int c2, int a1){
	int i,j,k;
	for(i = b1, j = c1, k = a1; i <= b2 && j <= c2;){
		if(B[i] <= C[j])
			A[k++] = B[i++];
		else
			A[k++] = C[j++];
	}
	for(;i<=b2;)
		A[k++] = B[i++];
	for(;j<=c2;)
		A[k++] = C[j++];
}
int main(){
	FILE *fp,*fp1;
	fp = fopen("1.txt","r");
	fp1 = fopen("result.txt","w");
	int B[NUM],C[PART],D[PART];
	int i,mid;
	int p,q;
	omp_set_num_threads(NUM_THREADS);
	for(i = 0;i < NUM;i++)
		fscanf(fp,"%d",B+i);
	#pragma omp parallel for private(i)
	for (i = 0; i < PART; ++i){
		C[i] = B[i];
		D[i] = B[i+PART];
	}
	#pragma omp parallel default(none) shared(C,D)
	{
        #pragma omp sections
        {
		    #pragma omp section
		    qsort(C, PART, sizeof(int), compare_ints);
		    #pragma omp section
		    qsort(D, PART, sizeof(int), compare_ints);
        }
	}
	mid = (C[PART1] + D[PART1])/2;
    p = search(C,mid,0,PART-1);
	q = search(D,mid,0,PART-1);
    #pragma omp parallel
	{
       #pragma omp sections
        {
           #pragma omp section
		    merge(C,D,B,0,p,0,q,0);
		    #pragma omp section
		    merge(C,D,B,p+1,PART-1,q+1,PART-1,p+q+2);

        }
    }
    fclose(fp);
	for(i = 0;i < NUM;i++)
		fprintf(fp1, "%d\n", B[i]);
    fclose(fp1);
	return 0;
}
