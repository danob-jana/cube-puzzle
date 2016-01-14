#!/usr/bin/env python

# length: 46
STARTING_CHAIN = [
    2, 4, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 2, 4, 2, 2, 2, 4, 3, 2,
    2, 2, 2, 2, 3, 3, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 2, 3, 2, 3, 2,
    4, 2, 2, 3, 2, 3
]

AXIS_X = 0
AXIS_Y = 1
AXIS_Z = 2


class PuzzleState:
    def initialize(self):
        self.chain_remaining = STARTING_CHAIN
        self.voxels_occupied = set([(0, 0, 0)])
        self.moves = [(AXIS_Z, 1)]
        self.current_voxel = (0, 0, 0)
        self.ranges = [[0, 0], [0, 0], [0, 0]]
        self.global_solve_state = {
            'depth': 0,
            'positions': 0
        }

    def copy(self):
        other = PuzzleState()
        other.chain_remaining = list(self.chain_remaining)
        other.voxels_occupied = set(self.voxels_occupied)
        other.moves = list(self.moves)
        other.current_voxel = self.current_voxel
        other.ranges = [list(x) for x in self.ranges]
        other.global_solve_state = self.global_solve_state
        return other

    def create_from_move(self, move):
        axis, sign = move

        if axis == self.moves[-1][0]:
            # cannot move same axis twice in a row
            return None

        result = self.copy()

        next_chain_length = result.chain_remaining[-1]

        # note that index zero is already placed; we start at 1.
        for s in range(1, next_chain_length):
            new_voxel = list(self.current_voxel)
            new_voxel[axis] += s * sign

            if new_voxel[axis] < result.ranges[axis][1] - 3:
                # outside of the 4x4 cube, too negative
                return None

            if new_voxel[axis] > result.ranges[axis][0] + 3:
                # outside of the 4x4 cube, too positive
                return None

            new_voxel = tuple(new_voxel)
            if new_voxel in result.voxels_occupied:
                # this voxel is occupied
                return None

            # ok this voxel is good
            result.voxels_occupied.add(new_voxel)
            if new_voxel[axis] < result.ranges[axis][0]:
                result.ranges[axis][0] = new_voxel[axis]
            if new_voxel[axis] > result.ranges[axis][1]:
                result.ranges[axis][1] = new_voxel[axis]
            result.current_voxel = new_voxel

        result.chain_remaining = result.chain_remaining[:-1]
        result.moves.append(move)

        return result

    def solve(self):
        if not self.chain_remaining:
            return self

        for axis in range(3):
            for sign in [1, -1]:
                new_cube = self.create_from_move((axis, sign))
                if new_cube:
                    self.global_solve_state['positions'] += 1
                    if self.global_solve_state['positions'] % 100000 == 0:
                        print('positions seen: {} (depth {})'.format(
                            self.global_solve_state['positions'],
                            self.global_solve_state['depth']))

                    self.global_solve_state['depth'] += 1
                    solution = new_cube.solve()
                    self.global_solve_state['depth'] -= 1

                    if solution:
                        return solution

        # no solution found from this spot
        return None

    def __repr__(self):
        axis_names = ['x', 'y', 'z']
        return '\n'.join([
            '{sign}{axis} ({length})'.format(
                sign='+' if sign > 0 else '-',
                axis=axis_names[axis],
                length=length)
            for (axis, sign), length
            in zip(self.moves[1:], STARTING_CHAIN[::-1])
        ])


def main():
    s = PuzzleState()
    s.initialize()
    s = s.solve()
    print('solution:\n{}'.format(s))


if __name__ == '__main__':
    main()
