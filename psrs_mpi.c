#include<stdio.h>
#include<mpi.h>
#include<stdlib.h>
#define NUM 1000
int compare_ints(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}
int Find_Max(int **mmStart,int *nPosition,int *recvLength,int group_size);
int main(int argc,char* argv[]){
    int group_size, my_rank;
    MPI_Status status;
    int A[NUM];
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &group_size);
    if(my_rank==0){
        FILE *fp;
        fp = fopen("1.txt","r");
        for(int i=0;i<NUM;i++)
            fscanf(fp,"%d",A+i);
        fclose(fp);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int* C;
    int* D;
    int DataLength[group_size];
    int DataStart[group_size];
    int count = NUM/group_size;
    for(int i=0;i<group_size;i++){
        DataLength[i]=count;
        DataStart[i] = i*count;
    }
    DataLength[group_size-1] += (NUM%group_size);
    if(my_rank==0)
        MPI_Scatterv(A,DataLength,DataStart,MPI_INT,MPI_IN_PLACE,DataLength[my_rank],MPI_INT,0,MPI_COMM_WORLD);
    else

        MPI_Scatterv(A,DataLength,DataStart,MPI_INT,A,DataLength[my_rank],MPI_INT,0,MPI_COMM_WORLD);
   // for(int i=0;i<DataLength[my_rank];i++)
     //   printf("%d %d\n",my_rank,A[i]);
    qsort(A,DataLength[my_rank],sizeof(int),compare_ints);
    MPI_Barrier(MPI_COMM_WORLD);
    C = malloc(sizeof(int)*group_size);
    D = malloc(sizeof(int)*group_size*group_size);
   // for(int i=0;i<DataLength[my_rank];i++)
     //   printf("after:%d %d\n",my_rank,A[i]);
    for(int i=0;i<group_size;i++)
        D[i] = A[i*count/group_size];
    if(my_rank == 0)
        MPI_Gather(MPI_IN_PLACE,group_size,MPI_INT,D,group_size,MPI_INT,0,MPI_COMM_WORLD);
    else
        MPI_Gather(D,group_size,MPI_INT,D,group_size,MPI_INT,0,MPI_COMM_WORLD);
    if(my_rank == 0){
        qsort(D,group_size*group_size,sizeof(int),compare_ints);
     //   for(int i=0;i<group_size*group_size;i++)
       //     printf("point %d\n",D[i]);
        for(int i=0;i<group_size-1;i++)
            C[i] = D[(i+1)*group_size];
    }
    MPI_Bcast(C,group_size-1,MPI_INT,0,MPI_COMM_WORLD);
    int Start[group_size];
    int Length[group_size];
    int index=0;
    for(int i=0;i<group_size-1;i++){
        Start[i] = index;
        Length[i] = 0;
        while((index < DataLength[my_rank])&&(A[index]<=C[i])){
            Length[i]++;
            index++;
        }
    }
    Start[group_size-1]=index;
    Length[group_size-1] = DataLength[my_rank] - index;
    int recvbuffer[NUM];
    int recvLength[group_size];
    int recvStart[group_size];
    for(int i=0;i<group_size;i++){
        MPI_Gather(&Length[i],1,MPI_INT,recvLength,1,MPI_INT,i,MPI_COMM_WORLD);
        if(my_rank == i){
            recvStart[0] = 0;
            for(int j=1;j<group_size;j++)
                recvStart[j]=recvStart[j-1]+recvLength[j-1];
        }
        MPI_Gatherv(&A[Start[i]],Length[i],MPI_INT,recvbuffer,recvLength,recvStart,MPI_INT,i,MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int *mmStart[group_size];
    int nPosition[group_size];
    for(int i=0;i<group_size;i++){
        mmStart[i] = recvbuffer+recvStart[i];
        nPosition[i]=0;
    }
    int max;
    for(int i=0;;i++){
        max = Find_Max(mmStart,nPosition,recvLength,group_size);
        if(max==-1)
            break;
        else{
            A[i] = mmStart[max][nPosition[max]];
         //   printf("merging rank %d: max %d %d\n",my_rank,max,A[i]);
            nPosition[max]++;
        }
    }
    int mysendLength = recvStart[group_size-1]+recvLength[group_size-1];
    //for(int i=0;i<mysendLength;i++)
      //  printf("rank %d: %d\n",my_rank,recvbuffer[i]);
   // qsort(recvbuffer,mysendLength,sizeof(int),compare_ints);
    int sendLength[group_size];
    int sendStart[group_size];
    MPI_Gather(&mysendLength,1,MPI_INT,sendLength,1,MPI_INT,0,MPI_COMM_WORLD);
    if(my_rank==0){
        sendStart[0]=0;
        for(int i=1;i<group_size;i++){
            sendStart[i] = sendStart[i-1]+sendLength[i-1];
        }

    }
    int sortedData[NUM];
    MPI_Gatherv(A,mysendLength,MPI_INT,sortedData,sendLength,sendStart,MPI_INT,0,MPI_COMM_WORLD);
   if(my_rank==0){
        FILE *fp1;
        fp1 = fopen("3.txt","w");
        for(int i = 0;i < NUM;i++)
            fprintf(fp1, "%d\n", sortedData[i]);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}
int Find_Max(int **mmStart,int *nPosition,int *recvLength,int group_size){
    int min;
    int i,p;
    min = 2000000000;
    p = -1;
    for(i=0;i<group_size;i++){
        if(nPosition[i]<recvLength[i] && mmStart[i][nPosition[i]]<min){
            min = mmStart[i][nPosition[i]];
            p = i;
        }
    }
    return p;
}
