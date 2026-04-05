#include "response.h"
#include <string.h>

static uint8_t get_data(char *data, int8_t* speed)
{
	*speed = 0;
	uint8_t flag = 0;
	uint8_t len = strlen(data);
	for (uint8_t k = 0; k < len; k++){
		if (data[k] == '-'){
			if(flag) return 0;
			flag = 1;
			continue;
		}
		*speed = *speed * 10 + (data[k] - '0');
	}
	
	if(flag) *speed *= (-1);
	
	return 1;
}

static uint8_t get_info_cmd(char **argv, int8_t* speed_left, int8_t* speed_right){
	if(!get_data(argv[0], speed_left)) return 0;
	if(!get_data(argv[1], speed_right)) return 0;
	return 1;
}

uint8_t response_uart(char **argv, int8_t* speed_left, int8_t* speed_right, uint8_t* state){
	if(!get_info_cmd(argv, speed_left, speed_right)) return 0;
	if(*speed_left == 0 && *speed_right == 0) *state = 0;
	else *state = 1;
	if (*speed_left > 100 || *speed_right > 100 || *speed_left < -100 || *speed_right < -100)
		return 0;
	return 1;
}
