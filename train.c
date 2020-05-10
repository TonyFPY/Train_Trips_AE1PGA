// 20028407 scypf1 Pinyuan Feng
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define INFINITY 50000

// create a structure of a adjacency matrix of the graph
typedef struct adj_Graph
{
    char **vertex;
    int **edge;
    int vertex_num;
} graph;

int check(FILE *fp);
int count_ver(FILE *fp);
void create_graph_vertex(FILE *fp, graph *g);
void create_graph_edge(FILE *fp, graph *g);
char *input(const char *mesg);
/*
**Acknowledge: char *input(const char *mesg)
**this function is taken from  in the lecture (the prompt function).
**I change the variable names and use realloc funtion in this function.
*/
int find_pos(graph *g, char *place);
int cost(int sum, int num);
void Dijkstra(graph *g, int st, int en);
void free_vertex_memory(graph *g);
void free_edge_memory(graph *g);
void free_graph_memory(graph *g);

int main(int argc, char *argv[])
{

    FILE *file_open;
    // check the number of command line arguments
    if (argc != 2)
    {
        puts("Invalid command line arguments. Usage: train <disances.txt>");
        exit(4);
    }
    file_open = fopen(argv[1], "r");
    // check whether the file can be opened successfully
    if (file_open == NULL)
    {
        perror("Cannot open file.");
        exit(1);
    }

    // check the format of the file, especially the invalid characters
    if (check(file_open) == 2)
    {
        puts("Invalid distances file.");
        exit(2);
    }

    // read the file and create a  data-structure of the graph
    graph *G = (graph *)malloc(sizeof(graph));
    if (G == NULL)
    {
        puts("Unable to allocate memory.");
        exit(3);
    }
    // count the number of train stations
    rewind(file_open);
    G->vertex_num = count_ver(file_open);

    // create vertices and edges
    rewind(file_open);
    create_graph_vertex(file_open, G);
    create_graph_edge(file_open, G);

    // there is no routes if start and end are same.
    int a = 0;
    while (a < (G->vertex_num))
    {
        if (G->edge[a][a] != INFINITY)
        {
            free_graph_memory(G);
            puts("Invalid distances file.");
            exit(2);
        }
        a++;
    }

    // the main loop of the program
    int flag = 1;
    while (flag == 1)
    {
        // Input the start station
        char *start = input("Start station: ");
        if (start == NULL)
        {
            free_graph_memory(G);
            puts("Unable to allocate memory.");
            exit(3);
        }
        if (start[0] == '\0') // no input here
        {
            free(start);
            flag = 0; // quit the program
            continue;
        }
        int start_pos = find_pos(G, start);
        if (start_pos == -1) // the station does not exist;
        {
            puts("No such station.");
            free(start);
            continue;
        }

        // Input the end station
        char *end = input("End station: ");
        if (end == NULL)
        {
            puts("Unable to allocate memory.");
            free(start);
            free_graph_memory(G);
            exit(3);
        }
        int end_pos = find_pos(G, end);
        if (end_pos == -1) // the station does not exist;
        {
            puts("No such station.");
            free(start);
            free(end);
            continue;
        }
        if (strcmp(start, end) == 0) // two stations are same
        {
            puts("No journey, same start and end station.");
            free(start);
            free(end);
            continue;
        }

        // free the start and end spaces because they are no longer used in this time of loop
        // the start and end stations' indexes are obtained in the previous steps
        free(start);
        free(end);
        // Calculate the shortest distance with Dijkstra Algorithm
        Dijkstra(G, start_pos, end_pos);
    }

    // free memory if the instruction is out of the loop
    free_graph_memory(G);

    // close the file
    fclose(file_open);
    exit(0);
}

/*
**This function checks whether the file is invalid, which contains some value other than ',''\n'.
**The arguement is a pointer pointing to the file.
**The return value 1 means it is valid. Otherwise, return 2.
**Some other cases will be detected in some parts of other functions.
**e.g. the distance is zero, the distance between same station is not ',', etc.
**because it wastes space to detect all the possible cases in just one function.
*/
int check(FILE *fp)
{
    char ch;
    // check whether the top-left is ','
    fscanf(fp, "%c", &ch);
    if (ch != ',')
    {
        return 2;
    }

    // The first line of the file will be checked in create_graph_vertex function.
    int column_num = 0; // record how many ending statations in the first line
    while (ch != '\n')
    {
        if (ch == ',')
        {
            column_num++;
        }
        fscanf(fp, "%c", &ch);
    }

    /*
    **check the remaining lines
    **In particular, if a distance cell contains something other than
    **a positive non-zero integer or empty then it is invalid.
    **The case that the distance is zero will be detected in create_graph_edge function.
    */
    int row_num = 0; // record how many starting stations
    while (!feof(fp))
    {
        ch = fgetc(fp);
        // check whether the first character is '\n' or EOF,
        // becasue in the end of the file, there might be many '\n'
        if (ch == '\n' || ch == EOF)
        {
            break;
        }

        // before the ',', the string is the name of the station
        while (ch != ',' && ch != EOF)
        {
            ch = fgetc(fp);
        }

        // start to check value of each distance
        while (ch != '\n' && ch != EOF)
        {
            if ((ch = fgetc(fp)) == EOF)
            {
                row_num++;
                break;
            }
            if (ch == ',' || isdigit(ch))
            {
                continue;
            }
            else if (ch == '\n')
            {
                // it will jump out of the loop according to the loop condition
                row_num++;
                continue;
            }
            else
            {
                return 2;
            }
        }
    }

    // if the number of rows does not equal to the number of columns, then it is a invalid file
    if (row_num != column_num)
    {
        return 2;
    }
    return 1;
}

/*
**This function checks how many stations are there according to the first row.
**The arguement is a pointer pointing to the file.
**The return value is the number of stations.
*/
int count_ver(FILE *fp)
{
    int ch = fgetc(fp);
    int num = 0;
    while (ch != '\n')
    {
        // ',' seperates the different stations
        if (ch == ',')
        {
            num++;
            ch = fgetc(fp);
            continue;
        }
        ch = fgetc(fp);
    }
    return num;
}

/*
**This function finds the index of the station
**the arguement are a graph-type pointer and a pointer of the name of the station
**return the index if the station is found. Otherwise, return -1.
*/
int find_pos(graph *g, char *place)
{
    int i;
    for (i = 0; i < (g->vertex_num); i++)
    {
        // if name is same, return the index.
        if (strcmp(place, g->vertex[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

/*
**This function creates vertices of a graph.
**The arguement are a pointer pointing to the file and a graph-type pointer.
*/
void create_graph_vertex(FILE *fp, graph *g)
{
    // initialization
    int i = 0;
    int j = 0;
    int size = 10;
    int count;
    int ver_num = g->vertex_num;
    g->vertex = (char **)malloc(sizeof(char *) * ver_num);
    if (g->vertex == NULL)
    {
        puts("Unable to allocate memory.");
        exit(3);
    }
    for (count = 0; count < ver_num; count++)
    {
        g->vertex[count] = (char *)malloc(sizeof(char) * size);
        if (g->vertex[count] == NULL)
        {
            free(g->vertex);
            free(g);
            puts("Unable to allocate memory.");
            exit(3);
        }
    }

    // add vertex;
    char ch;
    fscanf(fp, "%c", &ch); // ch == the first ','
    while (i < ver_num)
    {
        fscanf(fp, "%c", &ch);
        if (ch == ',' || ch == '\n')
        {
            g->vertex[i++][j] = '\0';
            // Check if there are two consecutive ',' in the first row of the file
            if (j == 0)
            {
                free_vertex_memory(g);
                free(g);
                puts("Invalid distances file.");
                exit(2);
            }
            j = 0;
        }
        else
        {
            g->vertex[i][j++] = ch;
            if (j > size - 1)
            {
                size = size * 2;
                g->vertex[i] = (char *)realloc(g->vertex[i], sizeof(char) * size);
                if (g->vertex[i] == NULL)
                {
                    free_vertex_memory(g);
                    free(g);
                    puts("Unable to allocate memory.");
                    exit(3);
                }
            }
        }
    }
    return;
}

/*
**This function puts values into corresponding edges of a graph.
**The arguement are a pointer pointing to the file and a graph-type pointer.
*/
void create_graph_edge(FILE *fp, graph *g)
{
    //initialization
    int i = 0;
    int j = 0;
    int size = 10;
    int count;
    int ver_num = g->vertex_num;
    g->edge = (int **)malloc(sizeof(int *) * ver_num);
    if (g->edge == NULL)
    {
        free_vertex_memory(g);
        free(g);
        puts("Unable to allocate memory.");
        exit(3);
    }
    for (count = 0; count < ver_num; count++)
    {
        g->edge[count] = (int *)malloc(sizeof(int) * ver_num);
        if (g->edge[count] == NULL)
        {
            free(g->edge);
            free_vertex_memory(g);
            free(g);
            puts("Unable to allocate memory.");
            exit(3);
        }
    }

    //add edges
    char ch;
    for (i = 0; i < ver_num; i++)
    {
        //find the corresponding position
        //create a space to store names of stations
        char *column_place = (char *)malloc(size * sizeof(char));
        if (column_place == NULL)
        {
            free_graph_memory(g);
            puts("Unable to allocate memory.");
            exit(3);
        }

        // if row oreder is different from the column order,
        // put the corresponding info in column order.
        int q = 0;
        do
        {
            fscanf(fp, "%c", &ch);
            column_place[q++] = ch;
        } while (ch != ',');
        column_place[q - 1] = '\0';
        int t = find_pos(g, column_place);
        // t is corresponding index in the first row.
        // if there no corresponding city in the first row,
        // then it is a invalid file.
        if (t == -1)
        {
            puts("Invalid distances file.");
            exit(2);
        }
        free(column_place);

        // store the value of edge
        for (j = 0; j < ver_num; j++)
        {
            fscanf(fp, "%c", &ch);
            if (isdigit(ch))
            {
                // use buffer to collect digits from 0 to 9
                char *buffer = (char *)malloc(size * sizeof(char)); // In reality, the distance between two stations is limited.
                if (buffer == NULL)
                {
                    free_graph_memory(g);
                    puts("Unable to allocate memory.");
                    exit(3);
                }
                int rec = 0;
                while (ch != ',' && ch != '\n' && ch != EOF)
                {
                    buffer[rec++] = ch;
                    fscanf(fp, "%c", &ch);
                }
                g->edge[t][j] = atoi(buffer); // transfer string to integer
                free(buffer);

                // check if value is 0
                if (g->edge[t][j] == 0)
                {
                    free_graph_memory(g);
                    puts("Invalid distances file.");
                    exit(2);
                }
            }
            else // ch == ',' || ch == '\n' || ch == EOF
            {
                // the route dosen't exist
                g->edge[t][j] = INFINITY;
            }
        }
    }
    return;
}

/*
**This function receives a prompt from the main function and input the name of a station.
**The arguement is a message which needs printing.
**return a pointer pointing to the name,
**return NULL if dynamic memeory allocation fails.
*/
char *input(const char *mesg)
{
    char *station;
    int size = 10;
    station = malloc(sizeof(char) * size);
    if (station == NULL)
    {
        puts("Unable to allocate memory.");
        return NULL;
    }
    printf("%s", mesg);
    int pos = 0;
    char ch;
    do
    {
        scanf("%c", &ch);
        if (ch != '\n') // User did not press return/enter.
        {
            station[pos] = ch;
            pos++;
            if (pos > size - 1)
            {
                // expand
                size = size * 2;
                station = (char *)realloc(station, sizeof(char) * size);
                if (station == NULL)
                {
                    puts("Unable to allocate memory.");
                    return NULL;
                }
            }
        }
    } while (ch != '\n');
    station[pos] = '\0'; // terminate the string after the return/enter.
    return station;
}

/*
**This function calculates the cost of a journey.
**the arguement are the total distance of a journey and the number of passing stations.
**return the total cost of the journey.
*/
int cost(int sum, int num)
{
    float RMB;
    RMB = sum * 1.2 + num * 25;
    // calculate the round-up value
    if ((RMB / 1) < RMB)
    {
        return RMB / 1 + 1;
    }
    else
    {
        return (int)RMB;
    }
}

/*
**This function calculates the shortest path.
**The arguement are a graph-type pointer and the start and end station index.
*/
void Dijkstra(graph *g, int st, int en)
{
    // Check whether there is no possible journey,
    // which means one row or one column is all ',';
    int p = 0, q = 0;
    int empty_row = 0, empty_col = 0;
    while (p < (g->vertex_num) && q < (g->vertex_num))
    {
        if (g->edge[st][p++] == INFINITY)
        {
            empty_col++;
        }
        if (g->edge[q++][en] == INFINITY)
        {
            empty_row++;
        }
        if (empty_row == g->vertex_num || empty_col == g->vertex_num)
        {
            puts("No possible journey.");
            return;
        }
    }

    int i, j, m, from, d;
    int minimum;
    // record whether the station was gone through
    int *book = (int *)malloc(sizeof(int) * (g->vertex_num));
    if (book == NULL)
    {
        free_graph_memory(g);
        puts("Unable to allocate memory.");
        exit(3);
    }
    // record the minimum distance from a station
    int *distance = (int *)malloc(sizeof(int) * (g->vertex_num));
    if (distance == NULL)
    {
        free(book);
        free_graph_memory(g);
        puts("Unable to allocate memory.");
        exit(3);
    }
    // record the previous passed station
    int *previous = (int *)malloc(sizeof(int) * (g->vertex_num));
    if (previous == NULL)
    {
        free(book);
        free(distance);
        free_graph_memory(g);
        puts("Unable to allocate memory.");
        exit(3);
    }
    // record what stations were gone through
    int *path = (int *)malloc(sizeof(int) * (g->vertex_num));
    if (path == NULL)
    {
        free(book);
        free(distance);
        free(previous);
        free_graph_memory(g);;
        puts("Unable to allocate memory.");
        exit(3);
    }

    // initializtion of calculation
    for (i = 0; i < (g->vertex_num); i++)
    {
        book[i] = 0;
        previous[i] = -1;
        distance[i] = g->edge[st][i];
    }

    // intialize the condition of the starting station
    from = st;
    book[from] = 1;
    distance[from] = 0;

    // use Dijkstra algorithm
    while (book[en] == 0)
    {
        minimum = INFINITY;
        m = 0;
        for (i = 0; i < (g->vertex_num); i++)
        {
            d = distance[from] + g->edge[from][i];
            if (d < distance[i] && book[i] == 0)
            {
                // replacement
                distance[i] = d;
                previous[i] = from;
            }
            if (minimum > distance[i] && book[i] == 0)
            {
                // next station
                minimum = distance[i];
                m = i;
            }
        }
        from = m;
        book[from] = 1;
    }

    // record the path
    from = en;
    j = 0;
    while (from != -1)
    {
        path[j++] = from;
        from = previous[from];
    }
    int pass = j - 1;
    if (pass == 0) // there is no passing station
    {
        printf("From %s\ndirect\nTo %s\n", g->vertex[st], g->vertex[en]);
    }
    else
    {
        printf("From %s\nvia\n", g->vertex[st]);
        j--;
        for (; j > 0; j--)
        {
            printf("%s\n", g->vertex[path[j]]);
        }
        printf("To %s\n", g->vertex[en]);
    }
    printf("Distance %d km\nCost %d RMB\n", distance[en], cost(distance[en], pass));
    free(book);
    free(distance);
    free(previous);
    free(path);
    return;
}

/*
**This functon frese the memories which store vertices' info before system terminated.
**The arguement is a graph-type pointer.
*/
void free_vertex_memory(graph *g)
{
    int k;
    for (k = 0; k < g->vertex_num; k++)
    {
        free(g->vertex[k]);
    }
    free(g->vertex);
    return;
}

/*
**This function frees the memories which stores edges' info before system terminated.
**The arguement is a graph-type pointer.
*/
void free_edge_memory(graph *g)
{
    int k;
    for (k = 0; k < g->vertex_num; k++)
    {
        free(g->edge[k]);
    }
    free(g->edge);
    return;
}

/*
**This function frees the memories which stores the graph's info before system terminated.
**The arguement is a graph-type pointer.
*/
void free_graph_memory(graph *g)
{
    free_vertex_memory(g);
    free_edge_memory(g);
    free(g);
    return;
}
