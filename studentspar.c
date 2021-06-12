#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define T 8

typedef struct
{
    int min;
    int max;
    double median;
    double mean;
    double sd;
} city;

typedef struct
{
    int min;
    int max;
    double median;
    double mean;
    double sd;
    city *cities;
} region;

typedef struct
{
    int min;
    int max;
    double median;
    double mean;
    double sd;
    region *regions;
} country;

double median(int *vec, int size)
{
    if (size % 2)
    {
        return (double)vec[size / 2];
    }
    else
    {
        return ((double)vec[size / 2] + (double)vec[size / 2 - 1]) / 2;
    }
}

double mean(int *vec, int size)
{
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += vec[i];
    }
    return sum / size;
}

double sd(int *vec, int size, double mean)
{
    double sum;
    for (int i = 0; i < size; i++)
    {
        sum += pow(vec[i] - mean, 2);
    }
    return sqrt(sum / size);
}

int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[])
{
    FILE *file;
    int R, C, A, seed;
    int best_mean_region_index, best_city_index_i, best_city_index_j;
    int *grades;
    double time;
    double best_mean_region, best_mean_city;
    region *regions;
    country brasil;

    if (argc != 2)
    {
        printf("Wrong input!\n");
        printf("Usage: %s name_file.txt\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "rt");

    if (file == NULL)
    {
        printf("Couldn't open file!\n");
        return 2;
    }

    fscanf(file, "%d", &R);
    fscanf(file, "%d", &C);
    fscanf(file, "%d", &A);
    fscanf(file, "%d", &seed);

    fclose(file);

    regions = (region *)malloc(R * sizeof(region));
    for (int i = 0; i < R; i++)
    {
        regions[i].cities = (city *)malloc(C * sizeof(city));
    }

    brasil.regions = regions;

    srand(seed);

    grades = (int *)malloc(R * C * A * sizeof(int));

    for (int i = 0; i < R * C * A; i++)
    {
        grades[i] = rand() % 101;
    }

    time = omp_get_wtime();

//Paraleliza a ordenação das notas de cada cidade e os cálculos das informações referentes a cada cidade
#pragma omp parallel num_threads(T) shared(grades)
    {
#pragma omp for
        for (int k = 0; k < R * C; k++)
        {
            int region_id = (int)k / C;
            int city_offset = k % C;
            int city_id = k * A;
            qsort(&(grades[city_id]), A, sizeof(int), cmpfunc);
            regions[region_id].cities[city_offset].min = grades[city_id];
            regions[region_id].cities[city_offset].max = grades[city_id + A - 1];
            regions[region_id].cities[city_offset].median = median(&(grades[city_id]), A);
            regions[region_id].cities[city_offset].mean = mean(&(grades[city_id]), A);
            regions[region_id].cities[city_offset].sd = sd(&(grades[city_id]), A, regions[region_id].cities[city_offset].mean);
        }

#pragma omp for
        for (int i = 0; i < R; i++)
        {
            qsort(&(grades[i * (C * A)]), C * A, sizeof(int), cmpfunc);
            double sum = 0;
            regions[i].min = grades[i * C * A];
            regions[i].max = grades[i * C * A + C * A - 1];
            regions[i].median = median(&(grades[i * (C * A)]), C * A);
            for (int j = 0; j < C; j++)
            {
                sum += regions[i].cities[j].mean;
            }
            regions[i].mean = sum / C;
            regions[i].sd = sd(&(grades[i * (C * A)]), C * A, regions[i].mean);
        }
    }

    qsort(grades, R * C * A, sizeof(int), cmpfunc);

    brasil.min = grades[0];
    brasil.max = grades[R * C * A - 1];
    brasil.median = median(grades, R * C * A);

    double sum = 0;
//Paraleliza o cálculo da soma das médias de todo o país através de uma redução de soma.
#pragma omp parallel for num_threads(T) shared(brasil) reduction(+ : sum)
    for (int i = 0; i < R; i++)
    {
        sum += brasil.regions[i].mean;
    }

    brasil.mean = sum / R;
    brasil.sd = sd(grades, R * C * A, brasil.mean);

    best_mean_region = -1;
    best_mean_city = -1;

    for (int i = 0; i < R; i++)
    {

        if (regions[i].mean > best_mean_region)
        {
            best_mean_region_index = i;
            best_mean_region = regions[i].mean;
        }
       
    }

    for (int i = 0; i < C * R; i++)
    {
        int region_id = i / C;
        int city_offset = i % C;
        if (regions[region_id].cities[city_offset].mean > best_mean_city)
        {
            best_mean_city = regions[region_id].cities[city_offset].mean;
            best_city_index_i = region_id;
            best_city_index_j = city_offset;
        }
    }

    time = omp_get_wtime() - time;

    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            printf("Reg %d - Cid %d: menor: %d, maior: %d, mediana: %.2lf, media: %.2lf e DP: %.2lf\n",
                   i, j, regions[i].cities[j].min, regions[i].cities[j].max, regions[i].cities[j].median, regions[i].cities[j].mean,
                   regions[i].cities[j].sd);
        }
        printf("\n");
    }

    for (int i = 0; i < R; i++)
    {
        printf("Reg %d: menor: %d, maior: %d, mediana: %.2lf, media: %.2lf e DP: %.2lf\n",
               i, regions[i].min, regions[i].max, regions[i].median, regions[i].mean, regions[i].sd);
    }

    printf("\nBrasil: menor: %d, maior: %d, mediana: %.2lf, media: %.2f e DP: %.2lf\n",
           brasil.min, brasil.max, brasil.median, brasil.mean, brasil.sd);

    printf("\nMelhor regiao: Regiao %d\n", best_mean_region_index);

    printf("\nMelhor cidade: Regiao %d, Cidade %d\n", best_city_index_i, best_city_index_j);

    printf("\nTempo de resposta sem considerar E/S, em segundos: %.8fs\n", time);

    free(grades);
    for(int i=0; i<R; i++){
        free(regions[i].cities);
    }
    free(regions);

    return 0;
}