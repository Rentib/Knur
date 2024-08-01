#ifndef KNUR_H_
#define KNUR_H_

#include <stdint.h>

enum color { WHITE, BLACK, COLOR_NB };

enum piece_type { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB };

/* clang-format off */
enum piece {
	WHITE_PAWN   =  0, BLACK_PAWN   = 1,
	WHITE_KNIGHT =  4, BLACK_KNIGHT = 5,
	WHITE_BISHOP =  8, BLACK_BISHOP = 9,
	WHITE_ROOK   = 12, BLACK_ROOK   = 13,
	WHITE_QUEEN  = 16, BLACK_QUEEN  = 17,
	WHITE_KING   = 20, BLACK_KING   = 21,
	EMPTY        = 26,
};
/* clang-format on */

#define PIECE_TYPE(piece)       ((piece) / 4)
#define PIECE_COLOR(piece)      ((piece) % 4)
#define PIECE_MAKE(type, color) ((type) * 4 + (color))

/* clang-format off */
enum square {
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQUARE_NB, SQ_NONE,
};
/* clang-format on */

#define SQ_FILE(square) ((square) & 7)
#define SQ_RANK(square) ((square) >> 3)
#define SQ_FLIP(square) ((square) ^ 56)
#define SQ_STR(square)                                                         \
	((square) == SQ_NONE                                                   \
	     ? "NO"                                                            \
	     : (char[]){'a' + SQ_FILE(square), '8' - SQ_RANK(square), 0})

enum direction {
	NORTH = -8,
	SOUTH = +8,
	EAST = +1,
	WEST = -1,
	NORTH_EAST = NORTH + EAST,
	NORTH_WEST = NORTH + WEST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_NORTH = NORTH + NORTH,
	SOUTH_SOUTH = SOUTH + SOUTH,
};

typedef uint64_t u64;

/* move enum
 * 16 bits to encode
 * 00|00|000000|000000
 * pt|mt| from |  to
 * pt   - promotion piece type
 * mt   - move type
 * from - origin square
 * to   - destination square
 */
enum move : uint16_t {
	MOVE_NONE = 0,  /* move from SQ_A8 to SQ_A8 */
	MOVE_NULL = 65, /* move from SQ_B8 to SQ_B8 */
};

enum move_type {
	MT_NORMAL = 0 << 12,
	MT_CASTLE = 1 << 12,
	MT_ENPASSANT = 2 << 12,
	MT_PROMOTION = 3 << 12,
};

#define MOVE_TO(move)         ((enum square)(((move) >> 0) & 0x3F))
#define MOVE_FROM(move)       ((enum square)(((move) >> 6) & 0x3F))
#define MOVE_TYPE(move)       ((enum move_type)((move) & (3 << 12)))
#define MOVE_PROMOTION(move)  ((enum piece_type)((((move) >> 14) & 3) + KNIGHT))
#define MAKE_MOVE(from, to)   ((enum move)(((from) << 6) + (to)))
#define MAKE_CASTLE(from, to) ((enum move)(MT_CASTLE + MAKE_MOVE(from, to)))
#define MAKE_ENPASSANT(from, to)                                               \
	((enum move)(MT_ENPASSANT + MAKE_MOVE(from, to)))
#define MAKE_PROMOTION(from, to, pt)                                           \
	((enum move)((((pt) - KNIGHT) << 14) + MT_PROMOTION +                  \
		     MAKE_MOVE(from, to)))
#define MOVE_STR(move)                                                         \
	((move) == MOVE_NONE ? "none"                                          \
	 : (move) == MOVE_NULL                                                 \
	     ? "null"                                                          \
	     : (char[]){SQ_STR(MOVE_FROM(move))[0],                            \
			SQ_STR(MOVE_FROM(move))[1], SQ_STR(MOVE_TO(move))[0],  \
			SQ_STR(MOVE_TO(move))[1],                              \
			MOVE_TYPE(move) == MT_PROMOTION                        \
			    ? "nbrq"[MOVE_PROMOTION(move) - KNIGHT]            \
			    : '\0',                                            \
			'\0'})

#define ABS(X)    ((X) < 0 ? -(X) : (X))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define MAX_MOVES      (1024)
#define MAX_PLY        (64)
#define STALEMATE      (0)
#define CHECKMATE      (32000 + MAX_PLY)
#define UNKNOWN        (CHECKMATE + 1)
#define MATE_IN(n)     (CHECKMATE - (n))
#define MATED_IN(n)    (CHECKMATE + (n))
#define IS_MATE(score) (ABS(score) >= CHECKMATE - MAX_PLY)

#define INLINE static inline __attribute__((always_inline))

#endif /* KNUR_H_ */
