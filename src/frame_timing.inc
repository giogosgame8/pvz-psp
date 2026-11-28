enum {
    TIMING_BOARD = 1, TIMING_ACTIVE = 2, TIMING_PAUSED = 4, TIMING_WON = 8,
    TIMING_REWARD = 16, TIMING_COLLECTED = 32
};

typedef struct {
    int flags, row, column;
} TimingScene;

typedef struct {
    TimingScene scene;
    uint64_t time_us;
    uint32_t reads, waits;
    int target_fps;
} TimingFrame;

typedef struct {
    uint64_t total_us, peak_us;
    uint32_t samples, long_frames, reads, waits;
} TimingGroup;

enum {
    TIMING_PLAY, TIMING_CURSOR, TIMING_PAUSE, TIMING_VICTORY, TIMING_REVEAL,
    TIMING_PICKUP, TIMING_GROUPS
};

typedef struct {
    TimingFrame previous;
    TimingGroup groups[TIMING_GROUPS];
} FrameTiming;

static TimingScene timing_scene(void){
    TimingScene scene = {0};
    if(!ready || screen != 1 || help || gallery || board_exit_screen >= 0) return scene;
    scene.flags = TIMING_BOARD;
    if(paused) scene.flags |= TIMING_PAUSED;
    if(!paused && game.status == PLAYING) scene.flags |= TIMING_ACTIVE;
    if(game.status == WON){
        scene.flags |= TIMING_WON;
        if(game.level_flags & LEVEL_REWARD_COLLECTED) scene.flags |= TIMING_COLLECTED;
        else if(reward_delay >= 0 && reward_elapsed >= reward_delay) scene.flags |= TIMING_REWARD;
    }
    scene.row = game.cursor_row;
    scene.column = game.cursor_col;
    return scene;
}

static void timing_add(TimingGroup *group, uint64_t interval, unsigned limit, uint32_t reads, uint32_t waits){
    group->samples++;
    group->total_us += interval;
    if(interval > group->peak_us) group->peak_us = interval;
    if(interval > limit) group->long_frames++;
    group->reads += reads;
    group->waits += waits;
}

static void timing_present(FrameTiming *timing, TimingScene before, TimingFrame current){
    TimingFrame previous = timing->previous;
    timing->previous = current;
    if(!previous.target_fps || previous.target_fps != current.target_fps ||
       current.time_us < previous.time_us) return;
    if(!(previous.scene.flags & before.flags & current.scene.flags & TIMING_BOARD)) return;

    uint64_t interval = current.time_us - previous.time_us;
    unsigned limit = 1500000 / current.target_fps;
    uint32_t reads = current.reads - previous.reads, waits = current.waits - previous.waits;
    int now = current.scene.flags;
    int entered = now & ~before.flags & ~previous.scene.flags;
    if(previous.scene.flags & before.flags & now & TIMING_ACTIVE){
        timing_add(&timing->groups[TIMING_PLAY], interval, limit, reads, waits);
        if(before.row != current.scene.row || before.column != current.scene.column)
            timing_add(&timing->groups[TIMING_CURSOR], interval, limit, reads, waits);
    }
    if(entered & TIMING_PAUSED) timing_add(&timing->groups[TIMING_PAUSE], interval, limit, reads, waits);
    if(entered & TIMING_WON) timing_add(&timing->groups[TIMING_VICTORY], interval, limit, reads, waits);
    if(entered & TIMING_REWARD) timing_add(&timing->groups[TIMING_REVEAL], interval, limit, reads, waits);
    if(entered & TIMING_COLLECTED) timing_add(&timing->groups[TIMING_PICKUP], interval, limit, reads, waits);
}

static void timing_write(FILE *log, FrameTiming *timing){
    static const char *const labels[] = {
        "gameplay", "cursor move", "pause open", "victory", "reward reveal", "reward pickup"
    };
    int wrote = 0;
    for(int i = 0; log && i < TIMING_GROUPS; i++){
        TimingGroup *g = &timing->groups[i];
        if(!g->samples) continue;
        fprintf(log, "FRAME TIMING %s: %u frames, mean %.2f ms, peak %.2f ms, %u long frames, %u sync reads, %u cache waits\n",
                labels[i], (unsigned)g->samples, g->total_us / (1000.0 * g->samples),
                g->peak_us / 1000.0, (unsigned)g->long_frames, (unsigned)g->reads, (unsigned)g->waits);
        wrote = 1;
    }
    if(wrote) fflush(log);
    memset(timing, 0, sizeof(*timing));
}
