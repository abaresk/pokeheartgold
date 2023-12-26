#ifndef POKEHEARTGOLD_POKE_OVERLAY_H
#define POKEHEARTGOLD_POKE_OVERLAY_H

#define OVY_MAX_PER_REGION            (8)

typedef enum PMiOverlayRegion {
    OVY_REGION_MAIN,
    OVY_REGION_ITCM,
    OVY_REGION_DTCM,
    OVY_REGION_NUM,
} PMiOverlayRegion;

typedef enum PMOverlayLoadType {
    OVY_LOAD_NORMAL,
    OVY_LOAD_NOINIT,
    OVY_LOAD_ASYNC,
} PMOverlayLoadType;

typedef struct ActiveOverlays {
    FSOverlayID overlays[OVY_MAX_PER_REGION];
    int length;
} ActiveOverlays;

void UnloadOverlayByID(FSOverlayID ovyId);
BOOL HandleLoadOverlay(FSOverlayID ovyId, PMOverlayLoadType loadType);
ActiveOverlays GetActiveOverlays(PMiOverlayRegion region);

#endif //POKEHEARTGOLD_POKE_OVERLAY_H
