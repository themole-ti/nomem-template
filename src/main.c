#define VDP_IMPLEMENTATION
#include "vdp.h"

#define INPUT_IMPLEMENTATION
#include "input.h"

#include "graphics.h"

int num_players = 1;

#define VDP_NT	0x0000
#define VDP_CT	0x0300
#define VDP_PT  0x0800
#define VDP_SA  0x0380
#define VPD_SP  0x1000

void init_graphics()
{
	// Prep registers
	VDP_SET_REGISTER(0, 0x00);			// No bitmap mode bit
	VDP_SET_REGISTER(1, VDP_MODE1_16K);	// Blank screen
	VDP_SET_REGISTER(2, 0x00);			// Name table at 0x0000 - 0x0300
	VDP_SET_REGISTER(3, 0x0c);			// Color table at 0x0300 - 0x0320
	VDP_SET_REGISTER(4, 0x01);			// Pattern definition table at 0x0800 - 0x1000
	VDP_SET_REGISTER(5, 0x07);			// Sprite attribute list at 0x0380 - 0x0480
	VDP_SET_REGISTER(6, 0x02);			// Sprite Pattern table at 0x1000 - 0x1800
	VDP_SET_REGISTER(7, 0x11);			// Black background, white foreground

	// Prep nametable
	vdpmemset(VDP_NT, 0, 768);

	// Prep color table
	vdpmemcpy(VDP_CT, colors, sizeof(colors));

	// Prep pattern table
	vdpmemcpy(VDP_PT, patterns, sizeof(patterns));

	// Upload sprite table
	vdpmemcpy(VPD_SP, sprite_patterns, sizeof(sprite_patterns));

	VDP_SET_REGISTER(1, VDP_MODE1_16K | VDP_MODE1_INT | VDP_MODE1_SPRMAG | VDP_MODE1_UNBLANK);
}

void show_intro()
{
	// Show Atari logo
	vdpmemset(VDP_NT, 0, 768);
	vdpmemcpy(VDP_NT + 128, nametable + 128, sizeof(nametable) - 128);

	// A little sleep has never hurt anyone
	for (int x = 0; x < 30; x++)
		VDP_WAIT_VBLANK;

	// Wait for fire
	int wait = 1;
	while (wait)
	{
		unsigned int result = read_joyst(JOYST_1);
		if (result & JOYST_FIRE)
			wait = 0;

		if (result & JOYST_UP)
		{
			num_players = 1;
			vdpchar(VDP_NT + 648, 50);
			vdpchar(VDP_NT + 710, 0);
		}

		if (result & JOYST_DOWN)
		{
			num_players = 2;
			vdpchar(VDP_NT + 648, 0);
			vdpchar(VDP_NT + 710, 50);
		}

		result = read_joyst(JOYST_2);
		if (result & JOYST_FIRE)
			wait = 0;

		if (result & JOYST_UP)
		{
			num_players = 1;
			vdpchar(VDP_NT + 648, 50);
			vdpchar(VDP_NT + 710, 0);
		}

		if (result & JOYST_DOWN)
		{
			num_players = 2;
			vdpchar(VDP_NT + 648, 0);
			vdpchar(VDP_NT + 710, 50);
		}
	}
}

void print_digit(int x, int y, int digit)
{
	int loc = (9 - digit) * 3;
	vdpmemcpy(x + (y + 0)*32, &nametable[loc +  0], 3);
	vdpmemcpy(x + (y + 1)*32, &nametable[loc + 32], 3);
	vdpmemcpy(x + (y + 2)*32, &nametable[loc + 64], 3);
	vdpmemcpy(x + (y + 3)*32, &nametable[loc + 96], 3);
}

void init_playfield()
{
	// Prep nametable
	vdpmemset(VDP_NT, 0, 768);

	// Print scores
	print_digit( 8, 0, 0);	
	print_digit(21, 0, 0);

	// Print 'net'
	for (int i = 0; i < 24; i++)
		vdpchar((32*i) + 16, 96);
}

void put_sprite(int spnum, int patt, int x, int y, int color)
{
	vdpchar(VDP_SA + (spnum * 4) + 0, y >> 4);
	vdpchar(VDP_SA + (spnum * 4) + 1, x >> 4);
	vdpchar(VDP_SA + (spnum * 4) + 2, patt);
	vdpchar(VDP_SA + (spnum * 4) + 3, color);
}

// Screen dimensions in "world coordinates" are: 4096x3072
// Paddle height is: 512
// Ball height is: 64
#define BALL_SPEED 1
#define BALL_ACCEL 6
#define PADDLE_SPEED 32

int ball_x  = 128 << 4;
int ball_y  =  96 << 4;
int ball_dx = BALL_SPEED << 4;
int ball_dy = BALL_SPEED << 4;

#define p1_x   (4 << 4)
int p1_y     = 96 << 4;
#define p2_x   (252 << 4) 
int p2_y     = 96 << 4;

int main(int argc, char *argv[])
{
	unsigned int thinking = 0;
	int ai_speed = 0;

	init_graphics();

	while(1)
	{
		show_intro();

		unsigned char p1_score = 0;
		unsigned char p2_score = 0;
		ball_dx = BALL_SPEED << 4;
		ball_dy = BALL_SPEED << 4;
		init_playfield();

		unsigned int result;
		while ((p1_score < 10) && (p2_score < 10))
		{
			// Player 1
			result = read_joyst(JOYST_1);

			if (result & JOYST_UP)
				p1_y -= PADDLE_SPEED;
			else if (result & JOYST_DOWN)
				p1_y += PADDLE_SPEED;

			// Player 2
			if (num_players == 1)
			{
				// Play against AI
				if ((ball_dx > 0) && (ball_x > 1024))
				{
					if (!thinking)
					{
						if (ball_x > 3584)
							thinking = 0;
						else
							thinking = 8;
						if (ball_y > (p2_y + 192))
							ai_speed =  PADDLE_SPEED;
						if (ball_y < p2_y)
							ai_speed =  -PADDLE_SPEED;
					}
					else
						thinking--;

					p2_y += ai_speed;
				}
			}
			else
			{
				// Play against real human
				unsigned int result = read_joyst(JOYST_2);

				if (result & JOYST_UP)
					p2_y -= PADDLE_SPEED;
				else if (result & JOYST_DOWN)
					p2_y += PADDLE_SPEED;
			}

			// Bounce of top and bottom
			if (p1_y < 0)
				p1_y = 0;
			if (p1_y > 2800)
				p1_y = 2800;

			if (p2_y < 0)
				p2_y = 0;
			if (p2_y > 2800)
				p2_y = 2800;

			// Check scoring wall (p1)
			if (ball_x < 128)
			{
				// Hit p1 paddle
				if (((p1_y + 288) > ball_y) && ((p1_y - 64) < ball_y))
				{
					if (ball_dx > 0)
						ball_dx = -ball_dx - BALL_ACCEL;
					else
						ball_dx = -ball_dx + BALL_ACCEL;
					ball_dy += BALL_ACCEL;

					// Make sure we don't fly past the paddle
					ball_x = 128;
				}
				else
				{
					p2_score++;
					print_digit(21, 0, p2_score);
					if (ball_dx < 0)
						ball_dx = -(BALL_SPEED << 4);
					else
						ball_dx = (BALL_SPEED << 4);
					ball_x = 2048;
					ball_y = 1536;
				}
			}

			// Check scoring wall (p2)
			if (ball_x > 3960)
			{
				// Hit p2 paddle
				if (((p2_y + 288) > ball_y) && ((p2_y - 64) < ball_y))
				{
					if (ball_dx < 0)
						ball_dx = -ball_dx + BALL_ACCEL;
					else
						ball_dx = -ball_dx - BALL_ACCEL;
					ball_dy += BALL_ACCEL;

					// Make sure we don't fly past the paddle
					ball_x = 3960;
				}
				else
				{
					p1_score++;
					print_digit(8, 0, p1_score);
					if (ball_dx < 0)
						ball_dx = -(BALL_SPEED << 4);
					else
						ball_dx = (BALL_SPEED << 4);
					ball_x = 2048;
					ball_y = 1536;
				}
			}

			if (ball_y < 0)
				ball_dy = -ball_dy;
			if (ball_y > 3000)
				ball_dy = -ball_dy;


			ball_x += ball_dx;
			ball_y += ball_dy;

			put_sprite(0, 4, ball_x, ball_y, 0xf);
			put_sprite(1, 0, p1_x,   p1_y, 0xf);
			put_sprite(2, 0, p2_x,   p2_y, 0xf);

			VDP_WAIT_VBLANK;
		}

		// Show final score
		if (p1_score == 10)
		{
			print_digit( 5, 0, 1);
			print_digit( 8, 0, 0);
		}
		else
		{
			print_digit(18, 0, 1);
			print_digit(21, 0, 0);
		}

		// Remove sprites
		put_sprite(0, 0, 0, 0, 0);
		put_sprite(1, 0, 0, 0, 0);
		put_sprite(2, 0, 0, 0, 0);

		// Wait for fire
		int wait = 1;
		while (wait)
		{
			result = read_joyst(JOYST_1);
			if (result & JOYST_FIRE)
				wait = 0;

			result = read_joyst(JOYST_2);
			if (result & JOYST_FIRE)
				wait = 0;

			VDP_WAIT_VBLANK;
		}
	}

	return 0;
}
