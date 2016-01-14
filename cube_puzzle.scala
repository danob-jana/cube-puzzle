
package org.danob.puzzle

import collection.mutable
import collection.immutable

sealed abstract class Axis
object Axis {
  case object X extends Axis
  case object Y extends Axis
  case object Z extends Axis
  val All = List(X, Y, Z)
}

sealed abstract class Direction(val sign:Int)
object Direction {
  case object Pos extends Direction(1) {
    override val toString = "+"
  }

  case object Neg extends Direction(-1) {
    override val toString = "-"
  }

  val All = List(Pos, Neg)
}

case class Move(axis:Axis, direction:Direction) {
  override val toString = direction.toString + axis.toString
}

object Move {
  val All:Iterable[Move] = Axis.All flatMap { axis ⇒
    Direction.All map { direction ⇒
      new Move(axis, direction)
    }
  }
}

case class Voxel(x:Int, y:Int, z:Int) {
  def apply(axis:Axis) = axis match {
    case Axis.X ⇒ x
    case Axis.Y ⇒ y
    case Axis.Z ⇒ z
  }

  def offset(axis:Axis, amount:Int) = axis match {
    case Axis.X ⇒ new Voxel(x+amount, y, z)
    case Axis.Y ⇒ new Voxel(x, y+amount, z)
    case Axis.Z ⇒ new Voxel(x, y, z+amount)
  }
}
case class SolvedException(state:PuzzleState) extends Exception
case class MoveNotAllowedException() extends Exception

class PuzzleState(
  val chainRemaining:Vector[Int],
  val voxelsOccupied:Set[Voxel],
  val moves:Vector[Move],
  val lastVoxel:Voxel,
  val ranges:immutable.Map[Axis, Tuple2[Int, Int]],
  val sharedSolveState:mutable.Map[String, Int]) {

  def with_move(move:Move):Option[PuzzleState] = {
    if (move.axis == moves.last.axis) {
      // cannot move same axis twice in a row
      return None
    }

    val next_chain_length = this.chainRemaining.head

    val new_voxels = new mutable.ArrayBuffer[Voxel]
    try {
      (1 until next_chain_length) foreach { s ⇒
        val new_voxel = lastVoxel.offset(move.axis, s * move.direction.sign)

        if (new_voxel(move.axis) < ranges(move.axis)._2 - 3) {
          // outside of the 4x4 cube, too negative
          throw new MoveNotAllowedException()
        }

        if (new_voxel(move.axis) > ranges(move.axis)._1 + 3) {
          // outside of the 4x4 cube, too positive
          throw new MoveNotAllowedException()
        }

        if (voxelsOccupied.contains(new_voxel)) {
          throw new MoveNotAllowedException()
        }

        // ok this voxel is good
        new_voxels.append(new_voxel)
      }
    } catch {
      case MoveNotAllowedException() ⇒ return None
    }

    val oldRange = this.ranges(move.axis)
    val newCoordinate = new_voxels.last(move.axis)
    val newRange = (
      if (newCoordinate < oldRange._1) newCoordinate else oldRange._1,
      if (newCoordinate > oldRange._2) newCoordinate else oldRange._2
    )

    Some(new PuzzleState(
      this.chainRemaining.tail,
      this.voxelsOccupied ++ new_voxels,
      moves :+ move,
      new_voxels.last,
      ranges + ((move.axis, newRange)),
      this.sharedSolveState
    ))
  }

  def solve():Option[PuzzleState] = {
    if (chainRemaining.isEmpty) {
      return Some(this)
    }

    try {
      Move.All foreach { move ⇒
        this.with_move(move) foreach { position ⇒
          sharedSolveState("positions") += 1
          if (sharedSolveState("positions") % 100000 == 0) {
            val p = sharedSolveState("positions")
            val d = sharedSolveState("depth")
            println(s"positions seen: $p (depth $d)")
          }

          sharedSolveState("depth") += 1
          position.solve() foreach { s:PuzzleState ⇒
            throw new SolvedException(s)
          }
          sharedSolveState("depth") -= 1
        }
      }
    } catch {
      case SolvedException(state) ⇒ return Some(state)
    }

    None
  }

  override def toString = {
    moves.tail mkString "\n"
  }
}

object PuzzleState {
  val startingChain = Vector(
    3, 2, 3, 2, 2, 4, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
    2, 2, 2, 2, 2, 3, 4, 2, 2, 2, 4, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    4, 2)

  val startingVoxel = Voxel(0, 0, 0)
  val startingVoxelsOccupied = Set(startingVoxel)
  val startingRanges:Map[Axis, Tuple2[Int, Int]] = Map(
    Axis.X → (0, 0),
    Axis.Y → (0, 0),
    Axis.Z → (0, 0))

  def apply() = new PuzzleState(
    startingChain,
    startingVoxelsOccupied,
    Vector[Move](new Move(Axis.Z, Direction.Pos)),
    startingVoxel,
    startingRanges,
    mutable.Map("positions" → 0, "depth" → 0)
  )
}

object Main extends App {
  val s = PuzzleState()
  val solution = s.solve()
  solution match {
    case Some(s) ⇒ println("solution: " + solution.toString)
    case None ⇒ println("no solution found!")
  }
}
