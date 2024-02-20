#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <graphics.h>
#include <curl/curl.h>
#include <math.h>

#define MAX_BUFFER_SIZE 1024
#define KARE_BOYUTU 10

struct Coordinate {
    int x;
    int y;
};

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        fprintf(stderr, "Bellek yetersiz (realloc NULL döndü)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void drawGrid(int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rectangle(j * KARE_BOYUTU, i * KARE_BOYUTU, (j + 1) * KARE_BOYUTU, (i + 1) * KARE_BOYUTU);
        }
    }
}

void drawLine(struct Coordinate p1, struct Coordinate p2) {

	
    setcolor(BLUE);
    setlinestyle(1,23,3);
    line(p1.x * KARE_BOYUTU, p1.y * KARE_BOYUTU, p2.x * KARE_BOYUTU, p2.y * KARE_BOYUTU);
    setcolor(WHITE);
}

double calculatePolygonArea(struct Coordinate *polygon, int numVertices) {
    double area = 0.0;

    for (int i = 0; i < numVertices; ++i) {
        int j = (i + 1) % numVertices;
        area += ((double)polygon[i].x * polygon[j].y - (double)polygon[j].x * polygon[i].y);
    }

    area = 0.5 * fabs(area);
    return area;
}


void drawCoordinateSet(struct Coordinate *list, int index) {
	int nokta1 = list[0].x;
	int nokta2 = list[0].y;
	int n=0;
    for (int i = 0; i < index - 1; i++) {
    	
        int sayac = 0;
        setlinestyle(1,23,3);
        line(list[n].x*10,list[n].y*10,list[n+1].x*10,list[n+1].y*10);
        n++;
        	
    	if(list[n].x == nokta1 && list[n].y == nokta2) {
			n++;
			nokta1 = list[n].x;
			nokta2 = list[n].y;
    		
		}		
    	delay(1000);
    }

    int points[2 * index];
    for (int i = 0; i < index; ++i) {
        points[2 * i] = list[i].x * KARE_BOYUTU;
        points[2 * i + 1] = list[i].y * KARE_BOYUTU;
        
    }

    setfillstyle(SOLID_FILL, BLUE);
    fillpoly(index, points);

    double area = calculatePolygonArea(list, index);
    double rezerv = area * 10;
    printf("\nYuzey Alani: %.2f br^2\n", area);
    printf("\nRezerv Degeri: %.2f br^2\n", rezerv);
}

void processFile(const char *filename, struct Coordinate *lists[], int *indexes[], int *numLists) {
    FILE *inputFile = fopen(filename, "r");

    if (inputFile == NULL) {
        fprintf(stderr, "Dosya açýlamadý.\n");
        return;
    }

    char buffer[MAX_BUFFER_SIZE];

    while (fgets(buffer, MAX_BUFFER_SIZE, inputFile) != NULL) {
    	if (strlen(buffer) <= 1) {
        // Skip empty lines
        continue;
    	}

    	printf("Koordinatlar: %s", buffer);

    	int setNumber;
    	if (sscanf(buffer, "%dB", &setNumber) == 1) {
        	*numLists += 1;
        	*indexes = (int *)realloc(*indexes, (*numLists) * sizeof(int));
        	(*indexes)[*numLists - 1] = 0;

        	*lists = (struct Coordinate *)realloc(*lists, (*numLists) * MAX_BUFFER_SIZE * sizeof(struct Coordinate));
    	}

    	if (*numLists == 0) {
        	continue;
    	}

    	int currentListIndex = *numLists - 1;
    	int currentIndex = (*indexes)[currentListIndex];

    	int start = 0;
    	int end = strlen(buffer);

    	int i = 0;
    	while (i < end && !isdigit(buffer[i])) {
        	++i;
    	}

    	if (i < end) {
        	start = i;
        	for (int j = start; j < end; ++j) {
            	if (buffer[j] == '(') {
                	struct Coordinate coord;
                	sscanf(&buffer[j], "(%d,%d)", &coord.x, &coord.y);

                	(*lists)[currentListIndex * MAX_BUFFER_SIZE + currentIndex] = coord;
                	currentIndex++;

                	if (currentIndex >= MAX_BUFFER_SIZE) {
                    	fprintf(stderr, "Hata: Koordinat listesi boyutu yetersiz.\n");
                    	break;
                	}
            	}
        	}
    	}

    	(*indexes)[currentListIndex] = currentIndex;
	}

    fclose(inputFile);
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    int gd = DETECT, gm;
    initgraph(&gd, &gm, "C:\\Turboc3\\BGI");

    int rows = getmaxy() / KARE_BOYUTU;
    int cols = getmaxx() / KARE_BOYUTU;

    drawGrid(rows, cols);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        struct MemoryStruct chunk;
        chunk.memory = (char *)malloc(1);
        chunk.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, "http://zplusorg.com/prolab.txt");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() basarisiz: %s\n", curl_easy_strerror(res));
        } else {
            FILE *outputFile = fopen("coordinates.txt", "w");
            if (outputFile == NULL) {
                fprintf(stderr, "Dosya açilamadi.\n");
                return 1;
            }

            fprintf(outputFile, "%s", chunk.memory);
            fclose(outputFile);

            struct Coordinate *coordinateLists = NULL;
            int *indexes = NULL;
            int numLists = 0;

            processFile("coordinates.txt", &coordinateLists, &indexes, &numLists);

            
            int choice;
            double birim_sondaj_maliyeti;
            
            while (1) {
                printf("\n\nHangi koordinatlari cizdirmek istiyorsunuz (1'den %d'e kadar bir sayi girin): ", numLists);
                scanf("%d", &choice);

                if (choice < 1 || choice > numLists) {
                    printf("Hatali secim. Lutfen gecerli bir sayi girin.\n");
                    continue;
                }

                printf("\nBirim sondaj maliyetini girin (1 ile 10 arasinda): ");
                scanf("%lf", &birim_sondaj_maliyeti);

                if (birim_sondaj_maliyeti < 1 || birim_sondaj_maliyeti > 10) {
                    printf("\nHatali birim sondaj maliyeti. Lütfen 1 ile 10 arasinda bir deger girin.\n");
                    continue;
                } else {
                    break;
                }
            }

            
            drawCoordinateSet(&coordinateLists[(choice - 1) * MAX_BUFFER_SIZE], indexes[choice - 1]);

            
            free(coordinateLists);
            free(indexes);
        }

        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    getch();
    closegraph();
    curl_global_cleanup();

    return 0;
}
