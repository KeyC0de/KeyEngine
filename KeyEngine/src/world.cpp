#include "world.h"


void World::update( const float dt )
{
}

void World::render( const float frameInterpolation )
{
}

/*
{
	const float dtFixed = 0.02f;
	auto oldTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float remainder = 0.0f;
	
	while(true)
	{
		float dt = m_gameTimer.lap() * settings.fGameSpeed / 1000.0f;
		
		auto accumulator = dt + remainder;
		while(accumulator > 0)
		{
			accumulator -= dtFixed;
			FixedUpdate(dtFixed);
		}
		remainder = accumulator;

		Update(dt);
		Present();
	}
}

constexpr const int fixedUpdatesPerSecond = 60;
constexpr const float dtFixed = 1.0f / fixedUpdatesPerSecond;
static float accumulator = 0.0f;
accumulator += dt;
accumulator = std::min(accumulator, 2.0f); // prevent huge deltas

loops = 0;
while (accumulator >= dtFixed && loops < maxFrameSkip)
{
	updateFixed( dtFixed );
	accumulator -= dtFixed;
	++loops;
}

frameInterpolation = float(accumulator / dtFixed);


void Engine::Run() {
	// Game updates at 60hz
	const float dt = 16.666f;

	float currentTime = 0.0f;
	float accumulator = 0.0f;

	while(m_isAlive)
	{
		CheckMessages();
		if(m_isActive)
		{
			SDL_Delay(1);

			float newTime = (float)SDL_GetTicks();
			float deltaTime = newTime - currentTime;
			currentTime = newTime;

			// Limit time to a maximum of 500 ms (.5 seconds)
			if(deltaTime > 500) {
				deltaTime = 500;
			}
			
			accumulator += deltaTime;
			
			// Step through the elapsed time in descreet time steps.
			while( accumulator >= dt) {
				
				// Do game stuff.
				m_input->Update();
				m_gsm->Update(dt);
				// Finished with game stuff.

				accumulator -= dt;
			}

			// Check to see if the Gamestate wants to quit.
			if(m_gsm->IsAlive() != true) {
				m_isAlive = false;
			}

			// Clear screen & Render
			m_graphics->ClearScreen();
			m_gsm->Render();
			m_graphics->Present();
		} else {
			// Game must be minimised/out of focus.
			SDL_Delay(10);
		}
	}
}



dt *= speed; // speed => speed-up factor, for our intents and purposes == 256.0f
{
	const auto maxDuration = std::min(1.0f, m_maxUpdateDuration * speed); // maxUpdateDuration == 0.2f
	if (dt > maxDuration)
		dt = maxDuration;
}

m_accumulator += dt;

while (m_accumulator >= m_timeStep)
{
	m_states.Tick(m_timeStep);
	
	m_accumulator -= m_timeStep;
}




In order to use the interpolation step (which by the way is totally optional) you have to keep two versions of your world-state: One for the current frame, and one for the last. Then, you would use the alpha-value in your drawing-routines to calculate the “now” position, based on current and last.

void draw(double alpha)
{
     Vector3 vNowPosition = lerp(vCurrentPosition, vLastPosition, alpha);
}
// you would interpolate between rotation, and scale when talking about transforms as well. Also there are lots of other things that you could interpolate too, like alpha-values for fades; colors when doing blend-effects, etc…
	same for drawing animation

This means copying over the state before you call your fixed update.

void prepareUpdate()
{
	vLastPosition = vCurrentPosition;
}
And if you really wanna do it right, you need have different methods for updating the position, depending on whether an object moves or teleports. In case of a teleport, you also need to assign your last-position; otherwise you end up with frames between current and target position (which is usually wrong).

*/