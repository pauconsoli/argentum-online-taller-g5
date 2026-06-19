#ifndef NPC_RENDERER_H
#define NPC_RENDERER_H

class NPCRenderer {
 private:
    static constexpr int BODY_W = 27;
    static constexpr int BODY_H = 48;
    static constexpr int HEAD_W = 27;
    static constexpr int HEAD_H = 64;


 public:
    void draw_npc(NPCSnapshot npc, Camera camera)
};

#endif
