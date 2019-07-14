#include "digits.h"

const std::array<digit_data, digit_count> digit =
{
	// 0
	digit_data::bitset{
		"111"
		"101"
		"101"
		"101"
		"111"
	},
	// 1
	digit_data::bitset{
		"010"
		"110"
		"010"
		"010"
		"010"
	},
	// 2
	digit_data::bitset{
		"111"
		"001"
		"111"
		"100"
		"111"
	},
	// 3
	digit_data::bitset{
		"111"
		"001"
		"111"
		"001"
		"111"
	},
	// 4
	digit_data::bitset{
		"100"
		"101"
		"111"
		"001"
		"001"
	},
	// 5
	digit_data::bitset{
		"111"
		"100"
		"111"
		"001"
		"111"
	},
	// 6
	digit_data::bitset{
		"110"
		"100"
		"111"
		"101"
		"111"
	},
	// 7
	digit_data::bitset{
		"111"
		"001"
		"001"
		"011"
		"001"
	},
	// 8
	digit_data::bitset{
		"111"
		"101"
		"111"
		"101"
		"111"
	},
	// 9
	digit_data::bitset{
		"111"
		"101"
		"111"
		"001"
		"011"
	},
	// :
	digit_data::bitset{
		"000"
		"010"
		"000"
		"010"
		"000"
	},
};
