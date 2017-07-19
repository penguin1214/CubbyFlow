#include "pch.h"

#include <Animation/Animation.h>

using namespace CubbyFlow;

TEST(Frame, Constructors)
{
	Frame frame;
	EXPECT_EQ(0u, frame.index);
	EXPECT_DOUBLE_EQ(1.0 / 60.0, frame.timeIntervalInSeconds);
}

TEST(Frame, TimeInSeconds)
{
	Frame frame;

	frame.index = 180;

	EXPECT_DOUBLE_EQ(3.0, frame.TimeInSeconds());
}

TEST(Frame, Advance)
{
	Frame frame;

	frame.index = 45;

	for (int i = 0; i < 9; ++i)
	{
		frame.Advance();
	}

	EXPECT_EQ(54u, frame.index);

	frame.Advance(23);

	EXPECT_EQ(77u, frame.index);

	EXPECT_EQ(78u, (++frame).index);

	EXPECT_EQ(78u, (frame++).index);

	EXPECT_EQ(79u, frame.index);
}