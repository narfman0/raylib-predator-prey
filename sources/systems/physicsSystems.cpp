#include "physicsSystems.h"

#include <box2d/box2d.h>
#include <spdlog/spdlog.h>

#include "systems.h"

// We'll store the world pointer in a static variable inside this translation
// unit. This keeps the interface simple.
static std::unique_ptr<b2World> g_world;
static float g_accumulator = 0.0f;

b2Body* createCircleBody(flecs::world &ecs, const Vector3 &position, float radius, bool isDynamic, flecs::entity owner) {
    if (!g_world) {
        spdlog::error("Physics world not initialized when creating body");
        return nullptr;
    }

    b2BodyDef bodyDef;
    bodyDef.type = isDynamic ? b2_dynamicBody : b2_staticBody;
    // Map 3D position.x,z to Box2D x,y (top-down)
    bodyDef.position.Set(position.x, position.z);
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(new flecs::entity(owner));

    b2Body* body = g_world->CreateBody(&bodyDef);

    b2CircleShape circle;
    circle.m_p.Set(0.0f, 0.0f);
    circle.m_radius = radius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.0f;

    body->CreateFixture(&fixtureDef);
    return body;
}

static void destroy_body_user_data(b2Body* body) {
    if (!body) return;
    // If we allocated a flecs::entity pointer for userData, free it.
    auto ud = body->GetUserData();
    if (ud.pointer) {
        auto p = reinterpret_cast<flecs::entity*>(ud.pointer);
        delete p;
    }
}

void initializePhysicsSystems(flecs::world &ecs) {
    // Create world with no gravity for top-down simulation
    g_world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));

    // Create system to step physics each frame. We'll use fixed timestep of 1/60
    const float timeStep = 1.0f / 60.0f;
    const int velocityIterations = 6;
    const int positionIterations = 2;

    ecs.system<>("Physics Step System")
        .kind(flecs::OnUpdate)
        .each([&](flecs::entity, flecs::iter& it) {
            float dt = GetFrameTime();
            // Simple accumulator fixed-step
            g_accumulator += dt;
            while (g_accumulator >= timeStep) {
                g_world->Step(timeStep, velocityIterations, positionIterations);
                g_accumulator -= timeStep;
            }

            // After stepping, write back positions to ECS Position components
            ecs.query<PhysicsBody, Position>().each([&](flecs::entity e, PhysicsBody &pb, Position &pos) {
                if (pb.body) {
                    b2Vec2 p = pb.body->GetPosition();
                    pos.x = p.x;
                    pos.z = p.y;
                    // Optionally update rotation if needed
                }
            });
        });

    // When a flecs entity with PhysicsBody is removed, destroy its Box2D body.
    ecs.observer<PhysicsBody>().event(flecs::OnDelete).each([&](flecs::entity e, PhysicsBody &pb) {
        if (pb.body && g_world) {
            destroy_body_user_data(pb.body);
            g_world->DestroyBody(pb.body);
            pb.body = nullptr;
        }
    });

    spdlog::info("Physics systems initialized");
}
*** End Patch