#include <json-c/json.h>
#include <stdio.h>

#include "evaluate.h"
#include "search.h"
#include "tuner.h"
#include "util.h"

#define ADD_PARAM(jobj, structure, field)                                      \
	do {                                                                   \
		json_object_object_add(jobj, #field,                           \
				       json_object_new_int(structure.field));  \
	} while (0)

#define ADD_PARAM_ARRAY(jobj, structure, field)                                \
	do {                                                                   \
		json_object *array = json_object_new_array();                  \
		for (unsigned i = 0; i < ARRAY_SIZE(structure.field); i++) {   \
			json_object_array_add(                                 \
			    array, json_object_new_int(structure.field[i]));   \
		}                                                              \
		json_object_object_add(jobj, #field, array);                   \
	} while (0)

void tuner_eval_json(void)
{
	json_object *jobj = json_object_new_object();

	ADD_PARAM_ARRAY(jobj, eval_params, piece_value);
	ADD_PARAM_ARRAY(jobj, eval_params, pawn_pcsqt);
	ADD_PARAM_ARRAY(jobj, eval_params, knight_pcsqt);
	ADD_PARAM_ARRAY(jobj, eval_params, bishop_pcsqt);
	ADD_PARAM_ARRAY(jobj, eval_params, rook_pcsqt);
	ADD_PARAM_ARRAY(jobj, eval_params, queen_pcsqt);
	ADD_PARAM_ARRAY(jobj, eval_params, king_pcsqt);

	ADD_PARAM(jobj, eval_params, pawn_backward);
	ADD_PARAM_ARRAY(jobj, eval_params, pawn_blocked);
	ADD_PARAM(jobj, eval_params, pawn_doubled);
	ADD_PARAM_ARRAY(jobj, eval_params, pawn_connected);
	ADD_PARAM(jobj, eval_params, pawn_isolated);
	ADD_PARAM_ARRAY(jobj, eval_params, pawn_passed);
	ADD_PARAM_ARRAY(jobj, eval_params, pawn_center);

	ADD_PARAM_ARRAY(jobj, eval_params, knight_adj);
	ADD_PARAM(jobj, eval_params, knight_outpost);

	ADD_PARAM(jobj, eval_params, bishop_pair);

	ADD_PARAM(jobj, eval_params, rook_connected);
	ADD_PARAM_ARRAY(jobj, eval_params, rook_adj);
	ADD_PARAM(jobj, eval_params, rook_open_file);
	ADD_PARAM(jobj, eval_params, rook_semiopen_file);
	ADD_PARAM(jobj, eval_params, rook_7th);

	printf("%s\n", json_object_to_json_string(jobj));
	json_object_put(jobj);
}

void tuner_search_json(void)
{
	json_object *jobj = json_object_new_object();

	ADD_PARAM(jobj, search_params, window_depth);
	ADD_PARAM(jobj, search_params, window_size);
	ADD_PARAM(jobj, search_params, rfp_depth);
	ADD_PARAM(jobj, search_params, rfp_margin);
	ADD_PARAM(jobj, search_params, nmp_depth);

	printf("%s\n", json_object_to_json_string(jobj));
	json_object_put(jobj);
}

#define SET_PARAM(jobj, structure, field)                                      \
	do {                                                                   \
		json_object *value = json_object_object_get(jobj, #field);     \
		if (value) {                                                   \
			structure.field = json_object_get_int(value);          \
		}                                                              \
	} while (0)

#define SET_PARAM_ARRAY(jobj, structure, field)                                \
	do {                                                                   \
		json_object *array = json_object_object_get(jobj, #field);     \
		if (array) {                                                   \
			for (unsigned i = 0; i < ARRAY_SIZE(structure.field);  \
			     i++) {                                            \
				json_object *value =                           \
				    json_object_array_get_idx(array, i);       \
				if (value) {                                   \
					structure.field[i] =                   \
					    json_object_get_int(value);        \
				}                                              \
			}                                                      \
		}                                                              \
	} while (0)

void tuner_eval_set(const char *json)
{
	json_object *jobj = json_tokener_parse(json);

	SET_PARAM_ARRAY(jobj, eval_params, piece_value);
	SET_PARAM_ARRAY(jobj, eval_params, pawn_pcsqt);
	SET_PARAM_ARRAY(jobj, eval_params, knight_pcsqt);
	SET_PARAM_ARRAY(jobj, eval_params, bishop_pcsqt);
	SET_PARAM_ARRAY(jobj, eval_params, rook_pcsqt);
	SET_PARAM_ARRAY(jobj, eval_params, queen_pcsqt);
	SET_PARAM_ARRAY(jobj, eval_params, king_pcsqt);

	SET_PARAM(jobj, eval_params, pawn_backward);
	SET_PARAM_ARRAY(jobj, eval_params, pawn_blocked);
	SET_PARAM(jobj, eval_params, pawn_doubled);
	SET_PARAM_ARRAY(jobj, eval_params, pawn_connected);
	SET_PARAM(jobj, eval_params, pawn_isolated);
	SET_PARAM_ARRAY(jobj, eval_params, pawn_passed);
	SET_PARAM_ARRAY(jobj, eval_params, pawn_center);

	SET_PARAM_ARRAY(jobj, eval_params, knight_adj);
	SET_PARAM(jobj, eval_params, knight_outpost);

	SET_PARAM(jobj, eval_params, bishop_pair);

	SET_PARAM(jobj, eval_params, rook_connected);
	SET_PARAM_ARRAY(jobj, eval_params, rook_adj);
	SET_PARAM(jobj, eval_params, rook_open_file);
	SET_PARAM(jobj, eval_params, rook_semiopen_file);
	SET_PARAM(jobj, eval_params, rook_7th);

	json_object_put(jobj);
}

void tuner_search_set(const char *json)
{
	json_object *jobj = json_tokener_parse(json);

	SET_PARAM(jobj, search_params, window_depth);
	SET_PARAM(jobj, search_params, window_size);
	SET_PARAM(jobj, search_params, rfp_depth);
	SET_PARAM(jobj, search_params, rfp_margin);
	SET_PARAM(jobj, search_params, nmp_depth);

	json_object_put(jobj);
}
