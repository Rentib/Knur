# Knur
UCI-compliant chess engine, written from scratch in C.

## Strength
Around 2400 elo.

## Features
- [Negamax](https://www.chessprogramming.org/Negamax) with [PVS](https://www.chessprogramming.org/Principal_Variation_Search)
- [Iterative Deepening](https://www.chessprogramming.org/Iterative_Deepening)
- [Quiescence Search](https://www.chessprogramming.org/Quiescence_Search)
- [Transposition Table](https://www.chessprogramming.org/Transposition_Table)
- [Aspiration Windows](https://www.chessprogramming.org/Aspiration_Windows)
- Move Ordering with [Staged Movegen](https://www.chessprogramming.org/Move_Generation#Staged_Move_Generation)
    - [Static Exchange Evaluation](https://www.chessprogramming.org/Static_Exchange_Evaluation)
    - [Killer Heuristic](https://www.chessprogramming.org/Killer_Heuristic)
    - [Countermove Heuristic](https://www.chessprogramming.org/Countermove_Heuristic)
- Pruning
    - [Mate Distance Pruning](https://www.chessprogramming.org/Mate_Distance_Pruning)
    - [Null Move Pruning](https://www.chessprogramming.org/Null_Move_Pruning)
    - [Reverse Futility Pruning](https://www.chessprogramming.org/Reverse_Futility_Pruning)
- Evaluation
    - [Tapered Eval](https://www.chessprogramming.org/Tapered_Eval)
    - [Pawn Hash Table](https://www.chessprogramming.org/Pawn_Hash_Table)

## Building
To build Knur run:
```bash
git clone https://github.com/Rentib/Knur
cd Knur/src
make
```

## Acknowledgements
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
- [Stockfish](https://github.com/official-stockfish/Stockfish/)
- [Berserk](https://github.com/jhonnold/berserk)
- [Ethereal](https://github.com/AndyGrant/Ethereal/tree/master)
