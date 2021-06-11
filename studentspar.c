#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

typedef struct{
    int min;
    int max;
    float median; 
    float mean;
    float sd;
}city;

typedef struct{
    int min;
    int max;
    float median; 
    float mean;
    float sd;
    city * cities;
}region;

typedef struct{
    int min;
    int max;
    float median;
    float mean;
    float sd;
    region * regions;
}country;



/*
int min_value(int * vec){
    return vec[0];
}

int max_value(int * vec, int size){
    return vec[size-1];
}
*/

float median(int * vec, int size){
    if(size%2){
        return (float) vec[size/2];
    }else{
        return ((float)vec[size/2] + (float)vec[size/2-1]) / 2;
    }
}

float mean(int * vec, int size){
    float sum = 0;
    for(int i=0; i<size; i++){
        sum += vec[i];
    }
    return sum / size;
}

float sd(int * vec, int size, float mean){
    double sum;
    for(int i=0; i<size; i++){
        sum += pow(vec[i]-mean,2);
    }
    return sqrt(sum / size);
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int main(int argc, char * argv[]){
    FILE * file;
    int R, C, A, seed;
    int best_mean_region_index, best_city_index_i, best_city_index_j;
    int *grades;
    double time;
    float best_mean_region, best_mean_city; 
    region * regions;
    country brasil;

    if(argc != 2){
        printf("Wrong input!\n");
        printf("Usage: %s name_file.txt\n",argv[0]);
        return 1;
    }

    file = fopen(argv[1],"rt");

    if(file == NULL){
        printf("Couldn't open file!\n");
        return 2;
    }

    fscanf(file, "%d", &R);
    fscanf(file, "%d", &C);
    fscanf(file, "%d", &A);
    fscanf(file, "%d", &seed);

    regions = (region*) malloc(R * sizeof(region));
    for(int i=0; i<R; i++){
        regions[i].cities = (city*) malloc(C * sizeof(city));
    }

    brasil.regions = regions;

    srand(seed);

    grades = (int*) malloc(R * C * A * sizeof(int));

    for(int i=0; i<R*C*A; i++){
        grades[i] = rand() % 101;
    }

    time = omp_get_wtime();

    #pragma omp parallel num_threads(8)
    {
        
    }

    for(int i=0; i<R; i++){
        for(int j=0; j<C; j++){
            qsort(&(grades[i*(C*A) + j*A]), A, sizeof(int), cmpfunc);
        }
    }

    for(int k=0; k<R*C; k++){
        int region_id = (int)k/C;
        int city_offset = k % C;
        regions[region_id].cities[city_offset].min = grades[region_id*(C*A) + city_offset*A];
        regions[region_id].cities[city_offset].max = grades[region_id*(C*A) + city_offset*A + A-1];
        regions[region_id].cities[city_offset].median = median(&(grades[region_id*(C*A) + city_offset*A]), A);
        regions[region_id].cities[city_offset].mean = mean(&(grades[region_id*(C*A) + city_offset*A]), A);
        regions[region_id].cities[city_offset].sd = sd(&(grades[region_id*(C*A) + city_offset*A]), A, regions[region_id].cities[city_offset].mean);

    }

/*
    for(int i=0; i<R; i++){
        for(int j=0; j<C; j++){
            printf("Reg %d - Cid %d: ", i, j);
            for(int k=0; k<A; k++){
                printf("%d ",grades[i*(C*A) + j*A + k]);
            }
            printf("\n");
        }
        printf("\n");
    }
*/
    for(int i=0; i<R; i++){
        qsort(&(grades[i*(C*A)]), C*A, sizeof(int), cmpfunc);
    }
/*
    for(int i=0; i<R; i++){
        for(int j=0; j<C; j++){
            printf("Reg %d - Cid %d: ", i, j);
            for(int k=0; k<A; k++){
                printf("%d ",grades[i*(C*A) + j*A + k]);
            }
            printf("\n");
        }
        printf("\n");
    }
*/

    for(int i=0; i<R; i++){
        float sum = 0;
        regions[i].min = grades[i*C*A];
        regions[i].max = grades[i*C*A + C*A-1];
        regions[i].median = median(&(grades[i*(C*A)]), C*A);
        for(int j=0; j<C; j++){
            sum += regions[i].cities[j].mean;
        }
        regions[i].mean = sum / C;
        regions[i].sd = sd(&(grades[i*(C*A)]), C*A, regions[i].mean);
    }

    qsort(grades, R*C*A, sizeof(int), cmpfunc);

    brasil.min = grades[0];
    brasil.max = grades[R*C*A-1];
    brasil.median = median(grades, R*C*A);
    float sum = 0;
    for(int i=0; i<R; i++){
        sum += brasil.regions[i].mean;
    }
    brasil.mean = sum / R;
    brasil.sd = sd(grades, R*C*A, brasil.mean);

    best_mean_region = -1;
    best_mean_city = -1;

    for(int i=0; i<R; i++){
        if(regions[i].mean > best_mean_region){
            best_mean_region = regions[i].mean;
            best_mean_region_index = i;
        }
        for(int j=0; j<C; j++){
            if(regions[i].cities[j].mean > best_mean_city){
                best_mean_city = regions[i].cities[j].mean;
                best_city_index_i = i;
                best_city_index_j = j;
            }
        }
    }

    time = omp_get_wtime() - time;

    for(int i=0; i<R; i++){
        for(int j=0; j<C; j++){
            printf("Reg %d - Cid %d: menor: %d, maior: %d, mediana: %.2f, media: %.2f e DP: %.2f\n",
            i, j, regions[i].cities[j].min, regions[i].cities[j].max, regions[i].cities[j].median, regions[i].cities[j].mean,
            regions[i].cities[j].sd);
        }
        printf("\n");
    }

    for(int i=0; i<R; i++){
        printf("Reg %d: menor: %d, maior: %d, mediana: %.2f, media: %.2f e DP: %.2f\n",
            i, regions[i].min, regions[i].max, regions[i].median, regions[i].mean, regions[i].sd);
    }

    printf("\nBrasil: menor: %d, maior: %d, mediana: %.2f, media: %.2f e DP: %.2f\n",
    brasil.min, brasil.max, brasil.median, brasil.mean, brasil.sd);

    printf("\nMelhor regiao: Regiao %d\n",best_mean_region_index);

    printf("\nMelhor cidade: Regiao %d, Cidade %d\n",best_city_index_i, best_city_index_j);

    printf("\nTempo de resposta sem considerar E/S, em segundos: %.8fs\n",time);
    
    return 0;
}