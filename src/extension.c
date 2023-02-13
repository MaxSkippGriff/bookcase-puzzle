#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Exit codes used for invalid files */
#define INVALID 2
#define VALIDFILE 2
#define INVALIDFILE 3
/* Maximum size of rows and columns */
#define MAXSIZE 9
/* Additional space required to realloc array */
#define EXTRA_SPACE 256
#define BUFFER_SIZE 512
/* The number of possible characters */
#define COLORS_LENGTH 9
/* Excludes the fullstop '.' character */
#define SHORTER_COLORS_LENGTH 8
/* Maximum number of children (MAXSIZE * (MAXSIZE-1)) */
#define TOO_DEEP 71
/* Output in the event of no solution */
#define NO_SOLUTION "No Solution?\n"

typedef enum bool {false, true} bool;
/* Bookcase structure */
typedef struct BC {
   struct BC *parent, *child;
   size_t rows, columns;
   char *books;
} BC;

/* Tests whether input is valid */
void valid_input(int argc);
/* Determines whether file is valid */
void valid_file(FILE *input_file);
/* Prints the bookcase according to content of file */
void print_bc(BC *bookcase, FILE *output_file, bool show_header);
/* Creates new bookcases */
void create_bc(size_t rows, size_t columns, FILE *input_file, char *books, BC *bookcase);
/* Destroys bookcases when required to do so */
void destroy_bc(BC *bookcase);
/* Ensures indexes are within bounds of the bookcase */
bool safe(size_t r, size_t c);
/* Parses input file */
BC *parse_file(FILE *input_file);
/* Clones bookcases */
BC *clone_bc(BC *bookcase);
/* Counts colors on rows */
bool count_colors_on_each_row(size_t *rows, size_t *columns, char *books);
/* Determines whether there are same colors in bookcase */
bool check_for_same_colors(int l);
/* Checks if the bookcase has been arranged so that it is 'happy' */
bool sort_bc(BC *bookcase);
/* Checks for repeated bookcases */
static bool repeated_bc(BC *bookcase_a, BC **array_master, size_t length_master);
/* Checks columns */
static size_t column_beyond_bc(BC *bookcase, size_t row);
/* Creates a new leaf that represents next move */
static BC *make_move(BC *leaf_bookcase, size_t x_a, size_t y_a,size_t x_b, size_t y_b);
/* Destroys bookcases if required */
static void memory_clean(size_t length_master, BC **array_master, BC **array_leaves, BC **array_next_leaves);
/* Prints solutions for verbose and non-verbose */
static void answer_found(BC *answer, FILE *output_file, bool verbose);
/* In the event there is no solution, returns "No solution?" */
static void no_solution(FILE *output_file);
/* Works out whether input can be solved */
static void solve_problem(BC *root_bookcase, FILE *output_file, bool verbose);

int main(int argc, char **argv)
{
   FILE *input_file;
   bool verbose;
   BC *root_bookcase;
   valid_input(argc);
   input_file = fopen(argv[1], "rb");
   valid_file(input_file);
   /* Verbose flag used */
   verbose = false;
   if(argc == 3){
    if(strcmp(argv[2], "verbose") == 0){
     verbose = true;
    }
   }
   root_bookcase = parse_file(input_file);
   if(root_bookcase == NULL){
    fprintf(stderr, "Failed to parse input file\r\n");
    exit(EXIT_FAILURE);
   }
   solve_problem(root_bookcase, stdout, verbose);
   fclose(input_file);
   return 0;
}

void valid_input(int argc)
{
   if(argc < VALIDFILE || argc > INVALIDFILE){
    fprintf(stderr,"Invalid input.\n");
    exit(EXIT_FAILURE);
  }
}

void valid_file(FILE *input_file)
{
   if(input_file == NULL){
    fprintf(stderr,"Invalid input.\n");
    exit(EXIT_FAILURE);
  }
}

void print_bc(BC *bookcase, FILE *output_file, bool show_header)
{
   size_t rows, columns, i, j;
   char *row_start;
   int character;
   if(show_header){
    fprintf(output_file,"%ld %ld\n", bookcase->rows, bookcase->columns);
   }
   rows = bookcase->rows;
   columns = bookcase->columns;
   for(i=0; i<rows; i+= 1){
    row_start = &bookcase->books[i * columns];
    for(j=0; j<columns; j+=1){
     character = (int) row_start[j];
     fputc(character, output_file);
    }
    fputs("\n", output_file);
   }
}

void destroy_bc(BC *bookcase)
{
   free(bookcase->books);
   free(bookcase);
}

void create_bc(size_t rows, size_t columns, FILE *input_file, char *books, BC *bookcase)
{
   size_t i, j, k;
   static const char COLORS[] = {'K', 'R', 'G', 'Y','B', 'M', 'C', 'W','.'};
   static char global_buffer[BUFFER_SIZE];
   char character;
   bool end_found, valid_color;
   char *row_start;
   for(i=0; i<rows; i+=1){
    fgets(global_buffer, BUFFER_SIZE, input_file);
    row_start = &books[i * columns];
    end_found = false;
    for(j=0; j<columns; j+=1){
     character = global_buffer[j];
     row_start[j] = character;
     if(character == '.'){
      end_found = true;
     }
     if(character != '.' && end_found){
      destroy_bc(bookcase);
      exit(EXIT_FAILURE);
     }
     valid_color = false;
     for(k=0; k<COLORS_LENGTH; k+=1){
      if(character == COLORS[k]){
       valid_color = true;
      }
     }
     if(!valid_color){
      destroy_bc(bookcase);
      exit(EXIT_FAILURE);
     }
    }
  }
}

BC *parse_file(FILE *input_file)
{
   size_t rows, columns, size_books;
   static char global_buffer[BUFFER_SIZE];
   BC *bookcase;
   char *books;
   int result = 0;
   fgets(global_buffer, BUFFER_SIZE, input_file);
   result = sscanf(global_buffer, "%ld %ld", &rows, &columns);
   if(result != 2){
    if(safe(rows, columns) == false){
     return NULL;
    }
   }
   bookcase = calloc(1, sizeof(*bookcase));
   /* Failed to allocate memory */
   if(bookcase == NULL){
    return NULL;
   }
   bookcase->parent = NULL;
   bookcase->child = NULL;
   bookcase->rows = rows;
   bookcase->columns = columns;
   size_books = sizeof(*bookcase->books) * rows * columns;
   books = malloc(size_books);
   if(books == NULL){
    free(bookcase);
    return NULL;
   }
   bookcase->books = books;
   create_bc(rows, columns, input_file, books, bookcase);
   return bookcase;
}

bool safe(size_t r, size_t c)
{
   if(r > MAXSIZE || c > MAXSIZE){
    return false;
   }
   return true;
}

BC *clone_bc(BC *bookcase)
{
   size_t rows, columns, books_size, s;
   char *books_a, *books_b;
   BC *clone = calloc(1, sizeof(*clone));
   clone->parent = bookcase;
   rows = bookcase->rows;
   columns = bookcase->columns;
   books_size = sizeof(*clone->books) * rows * columns;
   clone->rows = rows;
   clone->columns = columns;
   clone->books = malloc(books_size);
   books_a = clone->books;
   books_b = bookcase->books;
   for(s=0; s<books_size; s+=1){
    books_a[s] = books_b[s];
   }
   return clone;
}

bool count_colors_on_each_row(size_t *rows, size_t *columns, char *books)
{
   size_t i, j, k;
   static int global_colors_count[COLORS_LENGTH];
   static const char COLORS[] = {'K', 'R', 'G', 'Y','B', 'M', 'C', 'W','.'};
   char *row_start;
   char last_character, character;
   for(i=0; i < *rows; i+=1){
    row_start = &books[i * (*columns)];
    last_character = '.';
    for(j=0; j < *columns; j+=1){
     character = row_start[j];
      if(character == '.'){
        break;
      }
      if(last_character == '.'){
        last_character = character;
      }
      if(character != last_character){
       return false;
      }
     }/* Counts only rows that are not empty */
     if(last_character != '.'){
      for(k=0; k<COLORS_LENGTH; k+=1){
       if(COLORS[k] == last_character){
        global_colors_count[k] += 1;
       }
      }
    }
  } /* More than two rows with the same colours */
  return true;
}

bool check_for_same_colors(int l)
{
   static int global_colors_count[COLORS_LENGTH];
   for(l=0; l<SHORTER_COLORS_LENGTH; l+=1){
    if(global_colors_count[l] != 0 && global_colors_count[l] != 1){
     return false;
    }
  }
  return true;
}

bool sort_bc(BC *bookcase)
{
   size_t rows, columns;
   int i=0, l=0;
   char *books;
   static int global_colors_count[COLORS_LENGTH];
   for(i=0; i<COLORS_LENGTH; i+=1){
    global_colors_count[i] = 0;
   }
   rows = bookcase->rows;
   columns = bookcase->columns;
   books = bookcase->books;
   if(!count_colors_on_each_row(&rows, &columns, books)){
    return false;
   }
   if(!check_for_same_colors(l)){
    return false;
   }
   return true;
}

static bool repeated_bc(BC *bookcase_a, BC **array_master, size_t length_master)
{
   size_t i, j;
   char *books_a, *books_b;
   bool repeated = true;
   size_t books_size = sizeof(*bookcase_a->books) * bookcase_a->rows * bookcase_a->columns;
   for(i=0; i<length_master; i+=1){
    BC *bookcase_b = array_master[i];
    books_a = bookcase_a->books;
    books_b = bookcase_b->books;
    for(j=0; j<books_size; j+=1){
     if(books_a[j] != books_b[j]){
      repeated = false;
     }
    }
    if(repeated == true){
     return true;
    }
   }
   return false;
}

static size_t column_beyond_bc(BC *bookcase, size_t row)
{
   char character;
   size_t x = 0;
   size_t columns = bookcase->columns;
   char *books = bookcase->books;
   char *row_start = &books[row * columns];
   for(x=0; x<columns; x+=1){
    character = row_start[x];
    /* Early row end found */
    if(character == '.'){
     break;
    }
  }
  return x;
}

static BC *make_move(BC *leaf_bookcase, size_t x_a, size_t y_a, size_t x_b, size_t y_b)
{
   BC *next_leaf;
   char *books;
   char *row_start_a;
   char *row_start_b;
   char character_a, character_b;
   size_t columns = leaf_bookcase->columns;
   next_leaf = clone_bc(leaf_bookcase);
   books = next_leaf->books;
   row_start_a = &books[y_a * columns];
   row_start_b = &books[y_b * columns];
   character_a = row_start_a[x_a];
   character_b = row_start_b[x_b];
   row_start_a[x_a] = character_b;
   row_start_b[x_b] = character_a;
   return next_leaf;
}

static void memory_clean(size_t length_master, BC **array_master, BC **array_leaves, BC **array_next_leaves)
{
   size_t i;
   for(i=0; i<length_master; i+=1){
   BC *bookcase = array_master[i];
   destroy_bc(bookcase);
  }
  free(array_master);
  if(array_leaves != NULL){
   free(array_leaves);
  }
  if(array_next_leaves != NULL){
   free(array_next_leaves);
  }
}

static void answer_found(BC *answer, FILE *output_file, bool verbose)
{
   size_t ladder_steps = 1;
   int count = 1;
   while(answer->parent != NULL){
    answer->parent->child = answer;
    answer = answer->parent;
    ladder_steps += 1;
   } /* Shows the verbose output */
   while(answer != NULL){
    if(verbose){
     if(count== 1){
      fprintf(stdout, "%ld\n\n", ladder_steps);
      count--;
     }
     print_bc(answer, output_file, false);
     fputs("\n", stdout);
    }
    answer = answer->child;
   }
   if(!verbose){
    fprintf(stdout, "%ld\n", ladder_steps);
  }
}

static void no_solution(FILE *output_file)
{
   fprintf(output_file, NO_SOLUTION);
}

static void solve_problem(BC*root_bookcase, FILE *output_file, bool verbose)
{
   int depth = 0;
   BC **array_master;
   BC **array_leaves;
   BC **array_next_leaves;
   size_t i, j, k;
   size_t rows, columns;
   size_t x_a, y_a, x_b, y_b;
   size_t real_length_master;
   size_t length_master;
   size_t length_leaves;
   size_t real_length_next_leaves;
   size_t length_next_leaves;
   rows = root_bookcase->rows;
   columns = root_bookcase->columns;
   /* Array for storing all child nodes of possibilities */
   array_master = calloc(EXTRA_SPACE, sizeof(*array_master));
   real_length_master = EXTRA_SPACE;
   length_master = 0;
   /* Array for storing all children */
   array_leaves = calloc(EXTRA_SPACE, sizeof(*array_leaves));
   length_leaves = 0;
   /* Array for storing all *next* children */
   array_next_leaves = calloc(EXTRA_SPACE, sizeof(*array_next_leaves));
   real_length_next_leaves = EXTRA_SPACE;
   length_next_leaves = 0;
   array_master[0] = root_bookcase;
   length_master = 1;
   array_leaves[0] = root_bookcase;
   length_leaves = 1;
   while(true){
    depth += 1;
    if(depth == TOO_DEEP){
     no_solution(output_file);
     memory_clean(length_master, array_master, array_leaves, array_next_leaves);
     return;
    }
    /* Iterate through all children bookcases, creating new ones */
    for(i=0; i <length_leaves; i+=1){
     BC *leaf_bookcase;
     leaf_bookcase = array_leaves[i];
     if(sort_bc(leaf_bookcase)){
      answer_found(leaf_bookcase, stdout, verbose);
      memory_clean(length_master, array_master, array_leaves, array_next_leaves);
      return;
     }
     for(j=0; j<rows; j+=1){
      BC *next_leaf;
      bool is_repeated;
      for(k=0; k<rows; k+=1){
       /* No move when rows are the same */
       if(j == k){
        continue;
       }
       y_a = j;
       x_a = column_beyond_bc(leaf_bookcase, j);
       /* Empty row does not have an element to be moved */
       if(x_a == 0){
        continue;
       }
       x_a = x_a - 1;
       y_b = k;
       x_b = column_beyond_bc(leaf_bookcase, k);
       /* Row full, so ignore it */
       if(x_b == columns){
        continue;
       }
       next_leaf = make_move(leaf_bookcase, x_a, y_a, x_b, y_b);
       is_repeated = repeated_bc(next_leaf, array_master, length_master);
       if(is_repeated){
        destroy_bc(next_leaf);
        continue;
       }
       else{
        if(real_length_master == length_master){
         real_length_master += EXTRA_SPACE;
         array_master = realloc(array_master,real_length_master * sizeof(*array_master));
        }
       }/* Adds more space if necessary */
       if(real_length_next_leaves == length_next_leaves){
        real_length_next_leaves += EXTRA_SPACE;
        array_next_leaves = realloc(array_next_leaves,real_length_next_leaves * sizeof(*array_next_leaves));
       }
       array_next_leaves[length_next_leaves] = next_leaf;
       length_next_leaves += 1;
       array_master[length_master] = next_leaf;
       length_master += 1;
      }
     }
    } /* Releases the children */
    free(array_leaves);
    /* Now the "next" is the "current" */
    array_leaves = array_next_leaves;
    length_leaves = length_next_leaves;
    array_next_leaves = calloc(EXTRA_SPACE, sizeof(*array_next_leaves));
    real_length_next_leaves = EXTRA_SPACE;
    length_next_leaves = 0;
   }
}
