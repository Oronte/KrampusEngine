#include "SoftBodyComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"
#include "Graphics/Window/Window.h"

using namespace Krampus;

FVector2 SoftBodyComponent::ToWorld(const FVector2& local) const noexcept
{
    // Apply rotation if transform.rotation is in radians.
    // If your transform.rotation is in degrees, adapt: RotatedDeg(...)
    float rotationRad = owner->transform.rotation; // assume radians; adapt if needed
    FVector2 rotated = local.Rotated(rotationRad);
    return owner->transform.position + rotated;
}

FVector2 SoftBodyComponent::ToLocal(const FVector2& world) const noexcept
{
    float rotationRad = owner->transform.rotation;
    FVector2 local = world - owner->transform.position;
    // undo rotation
    return local.Rotated(-rotationRad);
}

/* -------- builders ---------- */

void SoftBodyComponent::BuildRing(int pointCount, float radius, float kstruct, float kshear, float kradial)
{
    particles.clear();
    springs.clear();

    kind = ShapeKind::Ring;
    ringCount = pointCount;
    ringRadius = radius;

    for (int i = 0; i < pointCount; ++i)
    {
        float ang = 2.f * 3.14159265358979323846f * float(i) / float(pointCount);
        FVector2 p{ std::cos(ang) * radius, std::sin(ang) * radius };
        Particle part;
        part.positionLocal = p;
        part.velocity = FVector2{ 0.f, 0.f };
        part.mass = 1.f;
        part.pinned = false;
        particles.push_back(part);
    }

    // structural springs
    for (int i = 0; i < pointCount; ++i)
    {
        int j = (i + 1) % pointCount;
        Spring s; s.a = i; s.b = j;
        s.restLength = (particles[i].positionLocal - particles[j].positionLocal).Length();
        s.k = kstruct; s.damping = 8.f;
        springs.push_back(s);
    }

    // shear (skip-1)
    for (int i = 0; i < pointCount; ++i)
    {
        int j = (i + 2) % pointCount;
        Spring s; s.a = i; s.b = j;
        s.restLength = (particles[i].positionLocal - particles[j].positionLocal).Length();
        s.k = kshear; s.damping = 5.f;
        springs.push_back(s);
    }

    // radial springs to center (b == -1 denotes center)
    for (int i = 0; i < pointCount; ++i)
    {
        Spring s; s.a = i; s.b = -1;
        s.restLength = particles[i].positionLocal.Length();
        s.k = kradial; s.damping = 6.f;
        springs.push_back(s);
    }
}

void SoftBodyComponent::BuildGrid(int width, int height, const FVector2& size, float kstruct, float kshear)
{
    particles.clear();
    springs.clear();

    kind = ShapeKind::Grid;
    gridW = width; gridH = height; gridSize = size;

    float sx = size.x / float(std::max(1, width - 1));
    float sy = size.y / float(std::max(1, height - 1));
    FVector2 offset{ -size.x * 0.5f, -size.y * 0.5f };

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Particle p;
            p.positionLocal = FVector2{ offset.x + x * sx, offset.y + y * sy };
            p.velocity = FVector2{ 0.f, 0.f };
            p.mass = 1.f;
            p.pinned = false;
            particles.push_back(p);
        }
    }

    auto idx = [width](int x, int y) { return y * width + x; };

    // structural springs
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (x + 1 < width)
            {
                int a = idx(x, y), b = idx(x + 1, y);
                Spring s; s.a = a; s.b = b;
                s.restLength = (particles[a].positionLocal - particles[b].positionLocal).Length();
                s.k = kstruct; s.damping = 6.f;
                springs.push_back(s);
            }
            if (y + 1 < height)
            {
                int a = idx(x, y), b = idx(x, y + 1);
                Spring s; s.a = a; s.b = b;
                s.restLength = (particles[a].positionLocal - particles[b].positionLocal).Length();
                s.k = kstruct; s.damping = 6.f;
                springs.push_back(s);
            }

            // diagonals
            if (x + 1 < width && y + 1 < height)
            {
                int a = idx(x, y), b = idx(x + 1, y + 1);
                Spring s; s.a = a; s.b = b;
                s.restLength = (particles[a].positionLocal - particles[b].positionLocal).Length();
                s.k = kshear; s.damping = 4.f;
                springs.push_back(s);
            }
            if (x - 1 >= 0 && y + 1 < height)
            {
                int a = idx(x, y), b = idx(x - 1, y + 1);
                Spring s; s.a = a; s.b = b;
                s.restLength = (particles[a].positionLocal - particles[b].positionLocal).Length();
                s.k = kshear; s.damping = 4.f;
                springs.push_back(s);
            }
        }
    }
}

/* ---------- constructors ---------- */

SoftBodyComponent::SoftBodyComponent(Actor* _owner, int pointCount, float radius, float kstruct, float kshear, float kradial)
    : Component(_owner)
{
    BuildRing(pointCount, radius, kstruct, kshear, kradial);
}

SoftBodyComponent::SoftBodyComponent(Actor* _owner, int width, int height, const FVector2& size, float kstruct, float kshear)
    : Component(_owner)
{
    BuildGrid(width, height, size, kstruct, kshear);
}

/* ---------- lifecycle ---------- */

void SoftBodyComponent::Construct()
{
    Component::Construct();
    owner->GetLevel()->GetCameraManager().AddToWindow(this);
}

void SoftBodyComponent::Deconstruct()
{
    Component::Deconstruct();
    owner->GetLevel()->GetCameraManager().RemoveToWindow(this);
}

/* ---------- simulation ---------- */

void SoftBodyComponent::Tick(const float& _deltaTime)
{
    Component::Tick(_deltaTime);

    // we run simulation with fixed dt and substeps for stability; accumulate time optionally
    float stepDt = dt / float(std::max(1, substeps));
    for (int sstep = 0; sstep < substeps; ++sstep)
    {
        std::vector<FVector2> forces(particles.size(), FVector2{ 0.f, 0.f });

        // gravity
        for (size_t i = 0; i < particles.size(); ++i)
        {
            if (!particles[i].pinned)
                forces[i] += gravity * particles[i].mass;
        }

        // springs
        for (const Spring& sp : springs)
        {
            FVector2 A_local = particles[sp.a].positionLocal;
            FVector2 B_local;
            bool bIsCenter = (sp.b == -1);
            if (bIsCenter) B_local = FVector2{ 0.f, 0.f };
            else B_local = particles[sp.b].positionLocal;

            // compute world-space delta (we use ToWorld to consider owner transform)
            FVector2 Aw = ToWorld(A_local);
            FVector2 Bw = ToWorld(B_local);
            FVector2 delta = Aw - Bw;
            float dist = delta.Length();
            if (dist <= 1e-8f) continue;
            FVector2 dir = delta / dist;
            float x = dist - sp.restLength;
            FVector2 Fworld = dir * (-sp.k * x);

            // damping along spring (project relative velocity on dir)
            FVector2 velA_world = particles[sp.a].velocity; // stored in local coords -> small simplification: velocities are local but since we use only translations + rotation they are acceptable
            FVector2 relV;
            if (!bIsCenter)
            {
                FVector2 velB_world = particles[sp.b].velocity;
                relV = velA_world - velB_world;
            }
            else
            {
                relV = velA_world; // center assumed pinned
            }
            float velAlong = relV.Dot(FVector2{ dir.x, dir.y });
            Fworld += dir * (-sp.damping * velAlong);

            // convert world force to local (since mass and velocity are in local space we simply use the force vector assuming no scaling)
            // small simplification: ignore rotation effect on forces (ok for small rotations or if transform rotation is rarely used)
            FVector2 Flocal = Fworld;

            if (!particles[sp.a].pinned) forces[sp.a] += Flocal;
            if (!bIsCenter && !particles[sp.b].pinned) forces[sp.b] -= Flocal;
        }

        // integrate semi-implicit Euler in local space
        for (size_t i = 0; i < particles.size(); ++i)
        {
            if (particles[i].pinned) continue;
            FVector2 accel = forces[i] / particles[i].mass;
            particles[i].velocity += accel * stepDt;
            particles[i].velocity *= globalDamping;
            particles[i].positionLocal += particles[i].velocity * stepDt;
        }

        // NOTE: boundary collisions / complex collisions are left to the user or to the physics system.
    }
}

/* ---------- drawing ---------- */

void SoftBodyComponent::Draw(Window& window)
{
    // Build Vertices and call window.Draw(vertices)
    // we don't include SFML here: Vertices wraps the sf::VertexArray internally
    if (drawFill && kind == ShapeKind::Ring && particles.size() >= 3)
    {
        Vertices poly(Krampus::PrimitiveType::Triangles);
        // simple fan triangulation around center in world space
        FVector2 center = owner->transform.position;
        // create triangle fan: center, p[i], p[i+1]
        for (size_t i = 0; i < particles.size(); ++i)
        {
            // center vertex
            poly.Append(center, fillColor, FVector2{ 0.f, 0.f });
            FVector2 a = ToWorld(particles[i].positionLocal);
            FVector2 b = ToWorld(particles[(i + 1) % particles.size()].positionLocal);
            poly.Append(a, fillColor, FVector2{ 0.f, 0.f });
            poly.Append(b, fillColor, FVector2{ 0.f, 0.f });
        }
        window.Draw(poly);
    }
    else if (drawFill && kind == ShapeKind::Grid && gridW >= 2 && gridH >= 2)
    {
        Vertices va(Krampus::PrimitiveType::Triangles);
        auto idx = [this](int x, int y) { return y * gridW + x; };
        for (int y = 0; y < gridH - 1; ++y)
        {
            for (int x = 0; x < gridW - 1; ++x)
            {
                FVector2 p00 = ToWorld(particles[idx(x, y)].positionLocal);
                FVector2 p10 = ToWorld(particles[idx(x + 1, y)].positionLocal);
                FVector2 p01 = ToWorld(particles[idx(x, y + 1)].positionLocal);
                FVector2 p11 = ToWorld(particles[idx(x + 1, y + 1)].positionLocal);

                va.Append(p00, fillColor); va.Append(p10, fillColor); va.Append(p01, fillColor);
                va.Append(p10, fillColor); va.Append(p11, fillColor); va.Append(p01, fillColor);
            }
        }
        window.Draw(va);
    }

    if (drawSprings)
    {
        Vertices lines(Krampus::PrimitiveType::Lines);
        for (const Spring& s : springs)
        {
            FVector2 A = ToWorld(particles[s.a].positionLocal);
            FVector2 B = (s.b == -1) ? ToWorld(FVector2{ 0.f, 0.f }) : ToWorld(particles[s.b].positionLocal);
            lines.Append(A, springColor); lines.Append(B, springColor);
        }
        window.Draw(lines);
    }

    // particles
    Vertices pts(Krampus::PrimitiveType::Points);
    for (const Particle& p : particles)
    {
        FVector2 wp = ToWorld(p.positionLocal);
        Color c = p.pinned ? Color::Red() : particleColor;
        pts.Append(wp, c);
    }
    window.Draw(pts);
}

/* ---------- interaction API ---------- */

bool SoftBodyComponent::GrabAtWorldPos(const FVector2& worldPos)
{
    float best = grabRadius;
    int besti = -1;
    for (size_t i = 0; i < particles.size(); ++i)
    {
        float d = (ToWorld(particles[i].positionLocal) - worldPos).Length();
        if (d < best) { best = d; besti = (int)i; }
    }
    if (besti != -1)
    {
        grabbedIndex = besti;
        return true;
    }
    return false;
}

void SoftBodyComponent::MoveGrabbedTo(const FVector2& worldPos)
{
    if (grabbedIndex < 0 || grabbedIndex >= (int)particles.size()) return;
    if (!particles[grabbedIndex].pinned)
    {
        particles[grabbedIndex].positionLocal = ToLocal(worldPos);
        particles[grabbedIndex].velocity = FVector2{ 0.f, 0.f };
    }
}

void SoftBodyComponent::ReleaseGrab()
{
    grabbedIndex = -1;
}

/* ---------- utility ---------- */

void SoftBodyComponent::PinParticle(int index, bool pinned)
{
    if (index < 0 || index >= (int)particles.size()) return;
    particles[index].pinned = pinned;
}