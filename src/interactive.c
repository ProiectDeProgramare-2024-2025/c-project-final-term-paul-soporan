#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct menu_option {
  char *label;
  void (*action)();
};

struct menu {
  char *title;
  char *header;
  struct menu_option *options;
  int option_count;
};

// TODO: Make it work on Windows.
void clear_screen() { system("clear"); }

void wait_for_enter() {
  printf("\nPress enter to continue...\n");

  while (getchar() != '\n') {
  }

  getchar();
}

void display_menu(struct menu *m, bool exit_option, bool display_after_action,
                  char *exit_label) {
  clear_screen();

  printf(COLOR("%s\n\n", BBLU), m->title);
  if (m->header) {
    printf("%s\n\n", m->header);
  }

  for (int i = 0; i < m->option_count; ++i) {
    printf(COLOR("%d.", GRN) " %s\n", i + 1, m->options[i].label);
  }

  if (exit_option) {
    printf(COLOR("0. ", RED));
    if (exit_label) {
      printf("%s\n", exit_label);
    } else {
      printf("Exit\n");
    }
  }

  printf("\n");

  int choice_lower_bound = exit_option ? 0 : 1;
  int choice_upper_bound = m->option_count;

  int choice;
  while (true) {
    printf("Enter your choice (%d-%d): ", choice_lower_bound,
           choice_upper_bound);
    scanf("%d", &choice);

    if (exit_option && choice == 0) {
      break;
    }

    if (choice < 1 || choice > m->option_count) {
      printf("Invalid choice. Please try again.\n");
      continue;
    }

    m->options[choice - 1].action();

    if (display_after_action) {
      display_menu(m, exit_option, display_after_action, exit_label);
    }

    break;
  }
}