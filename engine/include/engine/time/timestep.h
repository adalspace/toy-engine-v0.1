#ifndef TIME_TIMESTEP_H_
#define TIME_TIMESTEP_H_

namespace Core {
    class Timestep {
    public:
        Timestep(float time = 0.0f)
            : m_time(time) {}

        [[nodiscard]] float GetSeconds() const { return m_time; }
        [[nodiscard]] float GetMilliseconds() const { return m_time * 1000.f; }

        operator float() const { return m_time; }
    public:
        static Timestep FromMilliseconds(float milliseconds) { return Timestep(milliseconds * 0.001f); }
        static Timestep FromSeconds(float seconds) { return Timestep(seconds); }
    private:
        float m_time;
    };
}

#endif // TIME_TIMESTEP_H_