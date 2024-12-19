#pragma once
#include <gp_Dir.hxx>
const double PackSize = 252;

typedef public struct PackedNormal {
	unsigned char byte[2];
	PackedNormal(unsigned char u, unsigned char v) : byte{ u,v } { }
	
	unsigned char U() { return byte[0]; }
	unsigned char V() { return byte[1]; }

	static PackedNormal ToPackedNormal(const gp_Dir& vec);

	gp_Dir ToNormal();

} PackedNormal;

