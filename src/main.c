#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ansi.c"
#include "csv.c"
#include "interactive.c"

#define GAME_QUESTION_COUNT 15
#define ANSWER_COUNT 4

struct question {
  char *text;
  char *answers[ANSWER_COUNT];
  int correct_answer_idx;
};

struct question_entry {
  int question_idx;
  int answer_idx;
};

struct game_history {
  float score;
  struct question_entry entries[GAME_QUESTION_COUNT];
};

void question();

// Assumption: The index of a question never changes.

static struct question questions[100];
static int question_count = 0;

void load_questions() {
  FILE *fp = fopen("./data/questions.csv", "r");
  if (fp == NULL) {
    printf("Error opening questions.csv\n");
    return;
  }

  char buffer[1024];
  char **fields;
  short field_count;

  while (fgets(buffer, sizeof(buffer), fp)) {
    csv_parse_line(buffer, &fields, &field_count);

    questions[question_count].text = fields[0];
    for (int i = 0; i < ANSWER_COUNT; ++i) {
      questions[question_count].answers[i] = fields[i + 1];
    }
    questions[question_count].correct_answer_idx =
        atoi(fields[ANSWER_COUNT + 1]);

    free(fields);

    ++question_count;
  }
}

static int question_idx;
static float score = 0;

void update_game_history(char *name) {
  FILE *fp = fopen("./data/game_history.csv", "a");
  if (fp == NULL) {
    printf("Error opening game_history.csv\n");
    return;
  }

  time_t t = time(NULL);
  char *time_str = ctime(&t);

  fprintf(fp, "%s,%g,%s", name, score, time_str);

  fclose(fp);
}

void result() {
  clear_screen();

  printf("Congratulations!\nYour score: " COLOR(
             "%g", YEL) "/15\n\nPlease enter your name: ",
         score);

  char name[100];
  scanf("%s", name);

  update_game_history(name);

  printf(
      "\nThank you for playing, " COLOR("%s", BLU) "! Your score has been added to the "
      "leaderboard.\n",
      name);

  wait_for_enter();
}

void next_question() {
  wait_for_enter();

  if (++question_idx == GAME_QUESTION_COUNT) {
    result();
  } else {
    question();
  }
}

void correct_answer() {
  ++score;
  printf("Correct! You have received " COLOR("1", YEL) " point.\n");

  next_question();
}

void correct_answer_fifty_fifty() {
  score += 0.5;
  printf("Correct! You have received " COLOR("0.5", YEL) " points.\n");

  next_question();
}

void incorrect_answer() {
  printf("Incorrect! Your game is over.\n");

  wait_for_enter();

  result();
}

static int chosen_question_idx;

void fifty_fifty() {
  struct question q = questions[chosen_question_idx];

  int correct = q.correct_answer_idx;

  int other;
  do {
    other = rand() % ANSWER_COUNT;
  } while (other == correct);

  struct menu_option options[2];

  options[0].label = q.answers[correct];
  options[0].action = correct_answer_fifty_fifty;

  options[1].label = q.answers[other];
  options[1].action = incorrect_answer;

  char header[200];
  snprintf(
      header, sizeof(header),
      "Question " COLOR("%d", MAG) "/15 (Your score: " COLOR("%g", YEL) ")",
      question_idx + 1, score);

  struct menu m = {header, q.text, options,
                   sizeof(options) / sizeof(options[0])};

  display_menu(&m, true, false, "Abort game");
}

void question() {
  // TODO: Ensure that the same question is not repeated
  chosen_question_idx = rand() % question_count;

  struct question q = questions[chosen_question_idx];

  struct menu_option options[5];

  for (int i = 0; i < ANSWER_COUNT; ++i) {
    options[i].label = q.answers[i];
    options[i].action =
        i == q.correct_answer_idx ? correct_answer : incorrect_answer;
  }

  options[ANSWER_COUNT].label = "50/50 - remove two incorrect answers";
  options[ANSWER_COUNT].action = fifty_fifty;

  char header[200];
  snprintf(
      header, sizeof(header),
      "Question " COLOR("%d/15", MAG) " (Your score: " COLOR("%g", YEL) ")",
      question_idx + 1, score);

  struct menu m = {header, q.text, options,
                   sizeof(options) / sizeof(options[0])};

  display_menu(&m, true, false, "Abort game");
}

void question_view() {
  question_idx = 0;
  score = 0;

  question();
}

struct entry {
  char name[100];
  float max_score;
  char max_score_time[100];
};

int cmp_entries(const void *a, const void *b) {
  return ((struct entry *)b)->max_score - ((struct entry *)a)->max_score;
}

void leaderboard_view() {
  clear_screen();

  printf(COLOR("Leaderboard\n\n", BBLU));

  FILE *fp = fopen("./data/game_history.csv", "r");
  if (fp == NULL) {
    printf("Error opening game_history.csv\n");
    return;
  }

  struct entry entries[100];
  int entry_count = 0;

  int max_name_len = 0;

  char buffer[1024];
  char **fields;
  short field_count;

  while (fgets(buffer, sizeof(buffer), fp)) {
    csv_parse_line(buffer, &fields, &field_count);

    char *name = fields[0];
    float score = atof(fields[1]);
    char *time = fields[2];

    int name_len = strlen(name);
    if (name_len > max_name_len) {
      max_name_len = name_len;
    }

    // TODO: Implement a map instead of this... thing.
    bool found = false;
    for (int i = 0; i < entry_count; ++i) {
      if (strcmp(entries[i].name, name) == 0) {
        found = true;
        if (score > entries[i].max_score) {
          entries[i].max_score = score;
          strcpy(entries[i].max_score_time, time);
        }
        break;
      }
    }

    if (!found) {
      strcpy(entries[entry_count].name, name);
      entries[entry_count].max_score = score;
      strcpy(entries[entry_count].max_score_time, time);

      ++entry_count;
    }

    free(fields);
  }

  fclose(fp);

  qsort(entries, entry_count, sizeof(struct entry), cmp_entries);

  for (int i = 0; i < entry_count; ++i) {
    printf(COLOR("%d", RED) " | ", i + 1);
    printf(COLOR("%s%*s", BLU) " | ", entries[i].name, max_name_len - strlen(entries[i].name), "");
    if (entries[i].max_score == (int)entries[i].max_score) {
      printf(COLOR(" %g ", YEL), entries[i].max_score);
    } else {
      printf(COLOR("%g", YEL), entries[i].max_score);
    }

    printf(" | " COLOR("%s", GRN) "\n", entries[i].max_score_time);
  }

  wait_for_enter();
}

void user_history_view() {
  clear_screen();

  printf(COLOR("User History\n\n", BBLU));

  char name[100];
  printf("Enter your name: ");
  scanf("%s", name);

  FILE *fp = fopen("./data/game_history.csv", "r");
  if (fp == NULL) {
    printf("Error opening game_history.csv\n");
    return;
  }

  char buffer[1024];
  char **fields;
  short field_count;

  bool found = false;
  while (fgets(buffer, sizeof(buffer), fp)) {
    csv_parse_line(buffer, &fields, &field_count);

    if (strcmp(fields[0], name) == 0) {
      found = true;

      printf(COLOR("%s", BLU) " | ", fields[0]);
      if (strlen(fields[1]) == 1) {
        printf(COLOR(" %s ", YEL), fields[1]);
      } else {
        printf(COLOR("%s", YEL), fields[1]);
      }

      printf(" | " COLOR("%s", GRN) "\n", fields[2]);
    }

    free(fields);
  }

  if (!found) {
    printf("No stats found for " COLOR("%s", BLU) ".\n", name);
  }

  fclose(fp);

  wait_for_enter();
}

void main_menu() {
  struct menu_option play = {"Play", question_view};
  struct menu_option leaderboard = {"Leaderboard", leaderboard_view};
  struct menu_option stats = {"User History", user_history_view};

  struct menu_option options[] = {play, leaderboard, stats};

  struct menu m = {"Game: Who Wants to Be a Millionaire?", NULL, options,
                   sizeof(options) / sizeof(options[0])};

  display_menu(&m, true, true, NULL);
}

int main() {
  srand(time(NULL));

  load_questions();

  main_menu();

  return 0;
}