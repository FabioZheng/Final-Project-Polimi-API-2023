#include <stdio.h>
#include <stdlib.h>

#define MAXV 512
#define HASHSIZE 7727
#define MAXOVER 31
#define TOT 239537
#define MAX_LINE 6000

typedef struct station
{
    int dim;
    unsigned *cars;

} station_t;

typedef struct roadcell
{
    unsigned dist;
    unsigned maxCar;

} roadcell_t;

typedef struct hashnode
{
    int multiplier;
    station_t *station;
    struct hashnode *next;

} hashnode_t;


typedef struct queuenode
{
    unsigned val;
    struct queuenode *next;

} queuenode_t;


int getToken(char*line, int first, char*str, int* end){

    static int index=0;
    int num=0;

    if (first)
    {
        while (line[index]!=' ')
        {
            str[index]=line[index];
            index++;
        }

        str[index++]='\0';

        return 0;
    }

    while (line[index]!=' ' && line[index]!='\n')
    {
        num=num*10+line[index++]-'0';
    }
    
    if (line[index++]=='\n') 
    {
        index=0;
        *end=1;
    }

    return num;

}

hashnode_t *buildHashTable(hashnode_t*table, unsigned m)
{

    if (m==1 || table==NULL)
    {
        table = (hashnode_t *)malloc(sizeof(hashnode_t) * HASHSIZE);
    }
    else
    {
        table = (hashnode_t *)realloc(table, sizeof(hashnode_t)* HASHSIZE*m);
    }

    unsigned low=(m-1)*HASHSIZE, high=m*HASHSIZE;

    for (int i = low; i < high; i++)
    {
        table[i].next = NULL;
        table[i].multiplier = -1;
        table[i].station = NULL;
    }

    return table;
}



void listDealloc(hashnode_t *head)
{

    if (head->next == NULL)
    {
        free(head->station->cars);
        free(head->station);
        free(head);
        return;
    }
    listDealloc(head->next);
    free(head->station->cars);
    free(head->station);
    free(head);
}


void freeTable(hashnode_t *table, unsigned m)
{
    hashnode_t *temp = NULL;
    unsigned size=HASHSIZE*m;

    for (int i = 0; i < size; i++)
    {
        temp = &(table[i]);
        if (temp->next != NULL)
        {
            listDealloc(temp->next);
        }
        if (temp->station!=NULL)
        {
            free(temp->station->cars);
        }
        
        free(temp->station);
    }

    free(table);
}

hashnode_t *seek_and_destroy(hashnode_t *table, unsigned dist, int destroy, unsigned m)
{
    int mod = (dist % HASHSIZE)+(m-1)*HASHSIZE, mult = dist / HASHSIZE;

    hashnode_t *temp = &(table[mod]), *prev = NULL;

    do
    {
        if (temp->multiplier == mult)
        {

            if (destroy)
            {
                free(temp->station->cars);
                free(temp->station);
                temp->station = NULL;

                if (temp == &(table[mod]))
                {
                    temp->multiplier = -1;
                }
                else
                {
                    prev->next = temp->next;
                    free(temp);
                }

                return table;
            }

            return temp;
        }

        prev = temp;
        temp = temp->next;

    } while (temp != NULL);

    return NULL;
}

void enqueue(queuenode_t **queue, unsigned val, int reverse)
{

    queuenode_t *new, *temp = *queue;

    new = (queuenode_t *)malloc(sizeof(queuenode_t));
    new->val = val;
    new->next = NULL;

    if (temp == NULL)
    {
        *queue = new;
        return;
    }

    if (reverse)
    {
        *queue=new;
        new->next=temp;
    }
    else
    {   
        while (temp->next!=NULL)
        {
            temp=temp->next;
        }

        temp->next=new;
    }

}


void deallocQueue(queuenode_t**queue){

    queuenode_t*queuePtr=*queue, *prev;
    
    while (queuePtr!=NULL)
    {
        prev=queuePtr;
        queuePtr=queuePtr->next;
        free(prev);
    }

    *queue=NULL;    

}


int binarySearch(roadcell_t* A, int i, int j, int x, int reverse){

    int n=j-i+1;

    if (x<A[0].dist || x>A[j].dist) return -1;

    while (n>1)
    {
        if (x >= A[i + n / 2].dist) i=i + n / 2;

        else j=i + n / 2 - 1;

        n=j-i+1;
    }

    if (reverse && A[i].dist!=x)
    {
        i++;
    }
    

    return i;
    
}


void search(hashnode_t*source, hashnode_t*table, int start, int dest, int *found, unsigned *counter, queuenode_t **queue, unsigned m, int reverse, roadcell_t*road, int roadsize){

    unsigned maxTrip=0, curr=0, prevMax=0, *maxLength=NULL, lastst=0, curri=0, lastthr=0;
    char*reference=NULL;
    int start_roadIndex=binarySearch(road, 0, roadsize-1, start, 0), dest_roadIndex=binarySearch(road, 0, roadsize-1, dest, 0);

    if (source==NULL || source->station->cars==NULL)
    {
        return;
    }

    if (start==dest)
    {
        *found=1;
        return;
    }    

    

    if (reverse)
    {
        
        reference=(char*)malloc(sizeof(char)*(start_roadIndex-dest_roadIndex));
        maxLength=(unsigned*)malloc(sizeof(unsigned)*(start_roadIndex-dest_roadIndex));

        if (source->station->cars[0]>start)
        {
            maxTrip=0;
        }
        else
        {
            maxTrip = start - source->station->cars[0];
        }

        curr=start_roadIndex-1;

        while (maxTrip > dest)
        {
            prevMax = maxTrip;

            while (road[curr].dist > prevMax)
            {

                if (road[curr].dist-road[curr].maxCar < prevMax || road[curr].dist<road[curr].maxCar)
                {
                    break;
                }
                

                reference[curr - dest_roadIndex-1] = 'F';
                maxLength[curr - dest_roadIndex-1] = maxTrip;

                curr--;
            }

            while (road[curr].dist >= prevMax)
            {
                reference[curr - dest_roadIndex-1] = 'T';

                    
                if (road[curr].dist - road[curr].maxCar < maxTrip)
                {
                    maxTrip = road[curr].dist - road[curr].maxCar;
                }
                    
                else if (road[curr].maxCar>=road[curr].dist)
                {
                    maxTrip=0;
                }
                    
                

                maxLength[curr - dest_roadIndex-1] = maxTrip;

                lastst = curr;

                curr--;
            }

            reference[curr - dest_roadIndex] = 'A';

            if (maxTrip == prevMax)
            {   
                free(maxLength);
                free(reference);
                *counter = 0;
                return;
            }
        }


        while (road[curr].dist > dest)
        {
            reference[curr - dest_roadIndex-1] = 'F';
            maxLength[curr - dest_roadIndex-1] = 0;
            curr--;
        }

      
        for (int i = lastst; i < start_roadIndex; i++)
        {
            if (reference[i - dest_roadIndex-1] == 'F') continue;
            
            if (maxLength[i - dest_roadIndex-1] > dest && reference[binarySearch(road, 0, roadsize-1, maxLength[i - dest_roadIndex-1], reverse) - dest_roadIndex-1] == 'F')
            {
                reference[i - dest_roadIndex-1] = 'F';
            }
        }
        
        int fl=0;

        enqueue(queue, dest, reverse);

        curr = lastst - 1;


        while (curr < start_roadIndex)
        {
            curr++;
            if (fl && reference[curr-dest_roadIndex-1]!='A') continue;    
            
            if (reference[curr-dest_roadIndex-1]=='A') fl=0;
            
            if (maxLength[curr - dest_roadIndex-1] <= dest)
            {
                if ((road[curr].dist - road[curr].maxCar) <= dest || road[curr].maxCar > road[curr].dist)
                {
                    enqueue(queue, road[curr].dist, reverse);
                    fl=1;

                }
                    
            }
            else if (road[curr].dist-road[curr].maxCar<=(*queue)->val)
            {
                enqueue(queue, road[curr].dist, reverse);
                fl=1;
            }
        }

        enqueue(queue, start, reverse);       
        *found = 1;    
    }

    else
    {
        reference=(char*)malloc(sizeof(char)*(dest_roadIndex-start_roadIndex));
        maxLength=(unsigned*)malloc(sizeof(unsigned)*(dest_roadIndex-start_roadIndex));

        int signal=1;

        maxTrip = source->station->cars[0] + start;

        curr = start_roadIndex + 1;

        while (maxTrip < dest)
        {
            prevMax = maxTrip;

            while (road[curr].dist < prevMax)
            {
                if (road[curr].maxCar + road[curr].dist > prevMax)
                {
                    break;
                }


                reference[curr - start_roadIndex - 1] = 'F';
                maxLength[curr - start_roadIndex - 1] = maxTrip;

                curr++;
            }

            while (road[curr].dist <= prevMax)
            {   
                signal=0;
                reference[curr - start_roadIndex - 1] = 'T';

                if (road[curr].dist + road[curr].maxCar > maxTrip)
                {
                    maxTrip = road[curr].dist + road[curr].maxCar;
                }

                maxLength[curr - start_roadIndex - 1] = maxTrip;

                lastst = curr;

                curr++;
            }

            if (signal) curr++;
            

            reference[curr-start_roadIndex-2]='A';

            if (maxTrip == prevMax)
            {
                free(maxLength);
                free(reference);
                *counter = 0;
                return;
            }
        }

        while (road[curr].dist < dest)
        {
            reference[curr - start_roadIndex - 1] = 'F';
            maxLength[curr - start_roadIndex - 1] = 0;
            curr++;
        }

        int lastA=start_roadIndex;


        for (int i = lastst; i > start_roadIndex; i--)
        {
            if (reference[i - start_roadIndex - 1] == 'F')
                continue;

            if (reference[i - start_roadIndex - 1] == 'A')
            {
                lastA=i;
            }
        
            if (maxLength[i - start_roadIndex - 1] < dest && (reference[binarySearch(road, 0, roadsize-1, maxLength[i - start_roadIndex - 1], reverse) - start_roadIndex - 1] == 'F' ||  binarySearch(road, 0, roadsize-1, maxLength[i - start_roadIndex - 1], reverse)==lastA))
            {
                reference[i - start_roadIndex - 1] = 'F';
            }
        }

        curr = start_roadIndex;

        maxTrip = source->station->cars[0] + start;

        while (road[curr].dist <= maxTrip)
        {
            curr++;
            curri = curr;

            if (reference[curr - start_roadIndex - 1] == 'F') continue;

            enqueue(queue, road[curr].dist, reverse);

            lastthr = binarySearch(road, 0, roadsize-1, maxTrip, reverse);

            while (road[curri].dist < dest)
            {
                if (binarySearch(road, 0, roadsize-1, maxLength[curri - start_roadIndex - 1], reverse)<0)
                {
                    curri=dest_roadIndex;
                }
                else curri = binarySearch(road, 0, roadsize-1, maxLength[curri - start_roadIndex - 1], reverse);
                


                if (road[curri].dist == maxTrip) continue;

                int i = 0;

                for (i = curri; i > lastthr + 1 && reference[i - start_roadIndex - 2] == 'T'; i--);

                if (road[i].dist < dest)
                {
                    if (reference[i - start_roadIndex - 1] == 'F')
                    {
                        deallocQueue(queue);
                        break;
                    }

                    enqueue(queue, road[i].dist, reverse);
                }

                lastthr = curri;
            }

            if (road[curri].dist >= dest)
            {
                if (curr != curri)
                {   
                    enqueue(queue, dest, reverse);
                }

                *found = 1;
                break;
            }
        }
    }

    free(maxLength);
    free(reference);
}




int insertInTable(unsigned dist, unsigned veicoli, int *data, hashnode_t *table, unsigned m)
{

    int mod = dist % HASHSIZE, mult = dist / HASHSIZE, flag = 0;

    m--;

    hashnode_t *temp = &(table[mod+m*HASHSIZE]);

    if (temp->station != NULL)
    {
        flag = 1;
    }

    if (temp->multiplier == mult) return 0;

    while (temp->next != NULL)
    {
        temp = temp->next;
        if (temp->multiplier == mult) return 0;
    }

    if (flag)
    {
        temp->next = (hashnode_t *)malloc(sizeof(hashnode_t));
        temp = temp->next;
        temp->next = NULL;
    }
    else temp=&(table[mod+m*HASHSIZE]);

    temp->multiplier = mult;
    temp->station = (station_t *)malloc(sizeof(station_t));
    temp->station->cars = (unsigned *)malloc(sizeof(unsigned) * veicoli);
    temp->station->dim=veicoli;

    for (int i = 0; i < veicoli; ++i)
    {
        temp->station->cars[i] = data[i];
    }

    return 1;
}

int stringcomp(char *string1, char *string2)
{

    int i;

    for (i = 0; string1[i] != '\0'; i++)
    {
        if (string1[i] != string2[i])
        {
            return 0;
        }
    }

    if (string2[i] != '\0')
    {
        return 0;
    }

    return 1;
}


int *queueToList(queuenode_t*queue, unsigned*listsize, unsigned start, int reverse){
    queuenode_t*head=queue;
    int*list=NULL;
    unsigned size=1, index=1;

    if (reverse)
    {
        size=0;
        index=0;
    }

    while (head!=NULL)
    {
        size++;
        head=head->next;
    }

    *listsize=size;
    list=(int*)malloc(sizeof(int)*size);

    head=queue;
    
    if (!reverse)
    {
        list[0]=start;
    }
    
    while (head!=NULL)
    {
        list[index]=head->val;
        index++;

        head=head->next;
    }


    return list;
}


int *deleteDuplicate(int *list, unsigned* dim)
{

    int prev = list[0], newdim = 1, j = 0;
    int *newlist = NULL;

    for (int i = 1; i < *dim; i++)
    {
        if (list[i] == prev)
        {
            list[i] = -1;
            continue;
        }

        prev = list[i];
        newdim++;
    }

    newlist = (int *)malloc(sizeof(int) * newdim);

    for (int i = 0; i < *dim; i++)
    {
        if (list[i] != -1)
        {
            newlist[j] = list[i];
            j++;
        }
    }

    free(list);
    *dim=newdim;
    return newlist;
}



int addCar(unsigned dist, unsigned car, hashnode_t*table, unsigned m){

    hashnode_t*node=seek_and_destroy(table, dist, 0, m);
    int max=0, first=1;

    if (node==NULL)
    {
        return 0;
    }
    
    if (node->station->dim!=0)
    {
        max=node->station->cars[0];
        first=0;
    }


    int newdim=(node->station->dim)+1;

    node->station->cars=(unsigned*)realloc(node->station->cars, sizeof(unsigned)*newdim);

    if (node->station->cars==NULL)
    {
        printf("Reallocation Failed!\n");
        return 0;
    }
    
    if (first || car<max)
    {
        node->station->cars[newdim-1]=car;
    }
    else
    {
        node->station->cars[newdim-1]=max;
        node->station->cars[0]=car;
    }
    

    node->station->dim=newdim;

    return 1;

}

void swap_num(unsigned* a, unsigned* b) {
    unsigned temp = *a;

    *a = *b;
    *b = temp;
}


void heapify_num(unsigned *arr, int size, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < size && arr[left] > arr[largest])
        largest = left;

    if (right < size && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap_num(&arr[i], &arr[largest]);
        heapify_num(arr, size, largest);
    }
}


unsigned* heapSort_num(unsigned* list, int size, int rev) {

    for (int i = size / 2 - 1; i >= 0; i--) {
        heapify_num(list, size, i);
    }

    for (int i = size - 1; i > 0; i--) {
        swap_num(&list[0], &list[i]);
        heapify_num(list, i, 0);
    }

    if (rev)
    {
        unsigned*templist=NULL;
        templist=(unsigned*)malloc(sizeof(unsigned)*size);
        for (int i = 0; i < size; i++)
        {
            templist[i]=list[size-1-i];
        }

        free(list);

        return templist;     
    }
    return list;
}


int destroyCar(hashnode_t* table, unsigned dist, unsigned car, unsigned m){

    hashnode_t*nodeptr=seek_and_destroy(table, dist, 0, m);
    int dim, found=0, findex=-1;
    unsigned *temp;

    if (nodeptr==NULL)
    {
        return 0;
    }

    dim=nodeptr->station->dim;
    temp=nodeptr->station->cars;

    for (int i = 0; i < dim; i++)
    {
        if (!found && temp[i]==car)
        {
            findex=i;
            found=1;
            continue;
        }
        if (found)
        {
            temp[i-1]=temp[i];
        }
        
    }

    if (findex==0) nodeptr->station->cars=heapSort_num(temp, dim-1, 1);
    

    if (found)
    {
        nodeptr->station->cars=(unsigned*)realloc(nodeptr->station->cars, sizeof(unsigned)*(dim-1));
        nodeptr->station->dim=dim-1;
        return 1;
    }
    
    return 0;

}


void swap(roadcell_t* a, roadcell_t* b) {
    int temp_dist = a->dist, temp_maxCar=a->maxCar;

    a->dist = b->dist;
    a->maxCar=b->maxCar;
    b->dist = temp_dist;
    b->maxCar=temp_maxCar;
}


int partition(roadcell_t* arr, int low, int high) {
    int pivot = arr[high].dist;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (arr[j].dist <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }

    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

roadcell_t* quickSort(roadcell_t* arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }

    return arr;
}



roadcell_t* build_road(hashnode_t*table, unsigned*roadsize){

    hashnode_t*temp=NULL;
    roadcell_t*newroad=NULL;
    int j=0;

    for (int i = 0; i < HASHSIZE; i++)
    {
        temp=&table[i];

        while (temp!=NULL)
        {
            if (temp->multiplier>=0) (*roadsize)++;
            
            temp=temp->next;
        }
        
    }

    newroad=(roadcell_t*)malloc(sizeof(roadcell_t)*(*roadsize));
    
    for (int i = 0; i < HASHSIZE; i++)
    {
        temp=&table[i];

        while (temp!=NULL)
        {
            if (temp->multiplier>=0)
            {
                newroad[j].dist=i+HASHSIZE*temp->multiplier;
                if (temp->station->dim>0)
                {
                    newroad[j].maxCar=temp->station->cars[0];
                }
                else newroad[j].maxCar=0;
                
                j++;
            }
            
            temp=temp->next;
        }
    }

    return quickSort(newroad, 0, *roadsize-1);
}



int main()
{
    int k, veicoli=0, data[MAXV], imax;
    unsigned m=1, count=0, max, start, dest, car, dist=0;
    char line[MAX_LINE];
    char *aggiungi_stazione = "aggiungi-stazione", *trova = "pianifica-percorso", *demolisci = "demolisci-stazione", *aggiungi_auto = "aggiungi-auto", *rottama_auto = "rottama-auto";
    hashnode_t *table = NULL;
    queuenode_t *queue;

    table = buildHashTable(table, m);

    while (1)
    {   
        char split[30];
        int end=0;

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
            
        k = 0;
        queue=NULL;

        getToken(line, 1, split, &end);

        if (stringcomp(split, aggiungi_stazione))
        {
            
            max=0;
            imax=-1;

            if (count>TOT*m)
            {
                m++;
                table=buildHashTable(table, m);
            }

            while (!end)
            {
                switch (k)
                {
                case 0:
                    dist = getToken(line, 0, "", &end);
                    break;
                case 1:
                    veicoli = getToken(line, 0, "", &end);
                    break;
                default:

                    data[k-2] = getToken(line, 0, "", &end);

                    if (data[k-2]>max)
                    {
                        max=data[k-2];
                        imax=k-2;

                        if (imax!=0)
                        {
                            data[imax]=data[0];
                            data[0]=max;
                        }
                    }
                    
                    break;
                }

                k++;
            }

            if (insertInTable(dist, veicoli, data, table, m))
            {
                count++;
                printf("aggiunta\n");
            }
            else
            {
                printf("non aggiunta\n");
            }
            
        }

        else if (stringcomp(split, demolisci))
        {
            if (seek_and_destroy(table, getToken(line, 0, "", &end), 1, m) == NULL)
            {
                printf("non demolita\n");
            }
            else
            {
                count--;
                printf("demolita\n");
            }

        }

        else if (stringcomp(split, trova))
        {
            int found=0, *list=NULL;
            unsigned counter=1, reverse=0, listsize=0, roadsize=0;
            roadcell_t*road=NULL;
            hashnode_t*startNode=NULL;

            start=getToken(line, 0, "", &end);
            dest=getToken(line, 0, "", &end);

            if (start>dest)
            {
                reverse=1;
            }
            
            startNode=seek_and_destroy(table, start, 0, m);

            counter=1;

            road=build_road(table, &roadsize);

            search(startNode, table, start, dest, &found, &counter, &queue, m, reverse, road, roadsize);

            if (found)
            {

                list=queueToList(queue, &listsize, start, reverse);
                
                
                list=deleteDuplicate(list, &listsize);

                for (int i = 0; i < listsize; i++)
                {
                    if (i==listsize-1) printf("%d", list[i]);
                    
                    else printf("%d ", list[i]);
                    
                }
            }

            else if (!counter)
            {
                printf("nessun percorso");
            }

            printf("\n");

            free(road);
            free(list);
            deallocQueue(&queue);

        }

        else if (stringcomp(split, aggiungi_auto))
        {         
            dist=getToken(line, 0, "", &end);
            car=getToken(line, 0, "", &end);

            if (addCar(dist, car, table, m))
            {
                printf("aggiunta\n");
            }
            else
            {
                printf("non aggiunta\n");
            }
            
            
        }
        else if (stringcomp(split, rottama_auto))
        {
            dist=getToken(line, 0, "", &end);
            car=getToken(line, 0, "", &end);

            
            
            if (destroyCar(table, dist, car, m))
            {
                printf("rottamata\n");
            }
            else
            {
                printf("non rottamata\n");
            }
                 
        }


        else
        {
            printf("Incorrect command\n");
        }

    }

    freeTable(table, m);

    return 0;
}