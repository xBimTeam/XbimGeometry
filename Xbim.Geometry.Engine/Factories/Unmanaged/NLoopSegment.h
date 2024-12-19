#pragma once

struct NLoopSegment
{
public:
	int A;
	int B;
	NLoopSegment(int a, int b) : A(a), B(b) {}
	NLoopSegment(const NLoopSegment& segment) : A(segment.A), B(segment.B) {}
	bool operator==(const NLoopSegment& other) const
	{
		return (A == other.A
			&& B == other.B);
	}
	//bool operator<(NLoopSegment const& s) const { return A < s.A || (A == s.A && B < s.B); }
};

namespace std {

	template <>
	struct hash<NLoopSegment>
	{
		std::size_t operator()(const NLoopSegment& k) const
		{
			using std::size_t;
			using std::hash;
			using std::int32_t;
			return k.A ^ (k.B << 1);
		}
	};

}

bool operator<(const NLoopSegment& seg1, const NLoopSegment& seg2) { return seg1.A < seg2.A || (seg1.A == seg2.A && seg1.B < seg2.B); }
