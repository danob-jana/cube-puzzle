
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* length: 46 */
int32_t CHAIN[] = {
  3, 2, 3, 2, 2, 4, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
  2, 2, 2, 2, 2, 3, 4, 2, 2, 2, 4, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  4, 2
};

int32_t *CHAIN_END = &CHAIN[sizeof(CHAIN) / sizeof(int32_t)];

typedef struct {
  int32_t axis;
  int32_t direction;
  int32_t span_impact;
} PuzzleMove;

typedef struct {
  /*
    we store more space than we need in each direction -- and start in
    the center at voxels[3][3][3] -- because we don't know which
    direction we're going to end up moving. occupied voxels won't span
    more than 4.
  */
  int32_t voxels[7][7][7];

  /* 3 axes, a min and max for each */
  int32_t span[3][2];

  /*
    store the moves that got us here for the purpose of undoing.
  */
  int32_t move_count;
  PuzzleMove moves[sizeof(CHAIN) / sizeof(int32_t)];
  int32_t last_coordinates[3];

  /* remaining chain */
  int32_t *chain_remaining;

  int32_t solutions_found;
  int64_t positions_seen;
} PuzzleState;


void print_moves(PuzzleState* puzzle) {
  printf("moves: ");
  char* axes = "xyz";
  for (int32_t i = 0; i < puzzle->move_count; ++i) {
    printf("%s%c",
           puzzle->moves[i].direction < 0 ? "-" : "+",
           axes[puzzle->moves[i].axis]);
  }
  printf("\n");
}


void init_puzzle(PuzzleState *puzzle) {
  memset(puzzle, 0, sizeof(PuzzleState));
  puzzle->span[0][0] = 3;
  puzzle->span[0][1] = 3;
  puzzle->span[1][0] = 3;
  puzzle->span[1][1] = 3;
  puzzle->span[2][0] = 3;
  puzzle->span[2][1] = 3;
  puzzle->last_coordinates[0] = 3;
  puzzle->last_coordinates[1] = 3;
  puzzle->last_coordinates[2] = 3;
  puzzle->chain_remaining = &CHAIN[0];

  /* now make the first move */
  int32_t xmove = CHAIN[0]-1;
  for (int32_t i = 0; i <= xmove; ++i) {
    puzzle->voxels[i+3][3][3] = 1;
  }

  puzzle->span[0][1] += xmove;
  puzzle->last_coordinates[0] += xmove;
  puzzle->chain_remaining += 1;
  puzzle->move_count += 1;
  puzzle->moves[0].axis = 0;
  puzzle->moves[0].direction = 1;
  puzzle->moves[0].span_impact = xmove;

  /* now make the second move */
  int32_t ymove = CHAIN[1]-1;
  for (int32_t i = 0; i <= ymove; ++i) {
    puzzle->voxels[xmove+3][i+3][3] = 1;
  }

  puzzle->span[1][1] += ymove;
  puzzle->last_coordinates[1] += ymove;
  puzzle->chain_remaining += 1;
  puzzle->move_count += 1;
  puzzle->moves[1].axis = 1;
  puzzle->moves[1].direction = 1;
  puzzle->moves[1].span_impact = ymove;
}


inline int do_move(PuzzleState *puzzle, int32_t axis, int32_t direction) {
  /* cannot move same axis twice in a row */
  if (axis == puzzle->moves[puzzle->move_count-1].axis) {
    return 0;
  }

  int32_t next_chain_length = puzzle->chain_remaining[0];

  int32_t coords[3];
  coords[0] = puzzle->last_coordinates[0];
  coords[1] = puzzle->last_coordinates[1];
  coords[2] = puzzle->last_coordinates[2];

  int32_t span_impact = 0;

  if (direction < 0) {
    int32_t min = coords[axis] - next_chain_length + 1;
    if (min < puzzle->span[axis][1] - 3) {
      /* outside of the 4x4 cube -- too negative */
      return 0;
    }

    if (min < puzzle->span[axis][0]) {
      /* this will lower the min */
      span_impact = min - puzzle->span[axis][0];
    }
  } else {
    int32_t max = coords[axis] + next_chain_length - 1;
    if (max > puzzle->span[axis][0] + 3) {
      /* outside of the 4x4 cube -- too positive */
      return 0;
    }

    if (max > puzzle->span[axis][1]) {
      /* this will increase the max */
      span_impact = max - puzzle->span[axis][1];
    }
  }

  for (int32_t i = next_chain_length-1; i != 0; --i) {
    coords[axis] += direction;
    if (puzzle->voxels[coords[0]][coords[1]][coords[2]]) {
      /* this voxel is already occupied */
      return 0;
    }
  }

  /*
    ok, now we know that the move will succeed, we can start mutating
  */
  coords[0] = puzzle->last_coordinates[0];
  coords[1] = puzzle->last_coordinates[1];
  coords[2] = puzzle->last_coordinates[2];
  for (int32_t i = next_chain_length-1; i != 0; --i) {
    coords[axis] += direction;
    puzzle->voxels[coords[0]][coords[1]][coords[2]] = 1;
  }

  if (span_impact < 0) {
    puzzle->span[axis][0] = coords[axis];
  } else if (span_impact > 0) {
    puzzle->span[axis][1] = coords[axis];
  }

  puzzle->moves[puzzle->move_count].axis = axis;
  puzzle->moves[puzzle->move_count].direction = direction;
  puzzle->moves[puzzle->move_count].span_impact = span_impact;
  puzzle->move_count += 1;
  puzzle->last_coordinates[0] = coords[0];
  puzzle->last_coordinates[1] = coords[1];
  puzzle->last_coordinates[2] = coords[2];
  puzzle->chain_remaining += 1;

  return 1;
}


inline void undo_move(PuzzleState* puzzle) {
  PuzzleMove move = puzzle->moves[puzzle->move_count-1];
  puzzle->chain_remaining -= 1;

  int32_t coords[3];
  coords[0] = puzzle->last_coordinates[0];
  coords[1] = puzzle->last_coordinates[1];
  coords[2] = puzzle->last_coordinates[2];
  for (int32_t i = puzzle->chain_remaining[0]-1; i != 0; --i) {
    puzzle->voxels[coords[0]][coords[1]][coords[2]] = 0;
    coords[move.axis] -= move.direction;
  }

  puzzle->last_coordinates[0] = coords[0];
  puzzle->last_coordinates[1] = coords[1];
  puzzle->last_coordinates[2] = coords[2];
  puzzle->move_count -= 1;

  if (move.span_impact < 0) {
    puzzle->span[move.axis][0] -= move.span_impact;
  } else if (move.span_impact > 0) {
    puzzle->span[move.axis][1] -= move.span_impact;
  }
}


void find_solutions(PuzzleState* puzzle) {
  for (int32_t axis = 0; axis < 3; ++axis) {
    /* positive direction */
    if (do_move(puzzle, axis, 1)) {
      puzzle->positions_seen += 1;
      if (puzzle->chain_remaining >= CHAIN_END) {
        print_moves(puzzle);
        puzzle->solutions_found += 1;
      } else {
        find_solutions(puzzle);
      }

      undo_move(puzzle);
    }

    /* negative direction; only bother if there's been a move on this
       axis. by symmetry, we can find all solutions by just using the
       positive direction as the first move on each axis.
    */
    if (puzzle->span[axis][0] != puzzle->span[axis][1]) {
      if (do_move(puzzle, axis, -1)) {
        puzzle->positions_seen += 1;
        if (puzzle->chain_remaining >= CHAIN_END) {
          print_moves(puzzle);
          puzzle->solutions_found += 1;
        } else {
          find_solutions(puzzle);
        }

        undo_move(puzzle);
      }
    }
  }
}


int main() {
  PuzzleState puzzle;
  init_puzzle(&puzzle);

  clock_t start = clock();
  find_solutions(&puzzle);
  clock_t end = clock();
  double elapsed_time = (end-start)/(double)CLOCKS_PER_SEC;
  double nanoseconds_per_position = elapsed_time / puzzle.positions_seen * 1e9;

  printf("Took %.1f s\n", elapsed_time);
  printf("positions seen: %lld\n", puzzle.positions_seen);
  printf("(%.0fns/position)\n", nanoseconds_per_position);
  printf("solutions found: %d\n", puzzle.solutions_found);
}
