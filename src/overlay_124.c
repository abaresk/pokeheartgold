#include "bag_cursor.h"
#include "field_follow_poke.h"
#include "map_matrix.h"
#include "overlay_123.h"
#include "overlay_124.h"
#include "save_flypoints.h"
#include "unk_02092BE8.h"

FS_EXTERN_OVERLAY(OVY_123);

typedef struct {
    u8 unk0[8];
    SAVEDATA *savedata;
} Unk02260C40;

static void *ov124_02260D1C(FieldSystem *fsys);
static void ov124_02260D68(void);
static void ov124_02260D6C(void);
static void *ov124_02260D58(void);

void FieldSystem_init(OVY_MANAGER *man, FieldSystem *fsys) {
    int val;    
    u32 r4 = 0x0097b4b1;

    FS_LoadOverlay(0, FS_OVERLAY_ID(OVY_123));
    int ret = ov123_0225F4A8(ov124_02260D68);
    if (ret == 0) {
        val = 1;
    } else {
        val = 0;
    }
    r4 += val * 0x00000301;

    Unk02260C40 *ptr = OverlayManager_GetParentWork(man);
    fsys->savedata = ptr->savedata;
    fsys->taskman = NULL;
    
    int ret2 = ov123_0225F688(ov124_02260D6C);
    if (ret2 == 0) {
        val = 1;
    } else {
        val = 0;
    }
    r4 += val * 0x2f;

    fsys->location = FlyPoints_GetPosition(Save_FlyPoints_get(fsys->savedata));
    fsys->map_matrix = MapMatrix_New();

    int ret3 = ov123_0225F520(ov124_02260D58) * 0x000003a1;

    Field_AllocateMapEvents(fsys, 0xb);
    fsys->unk94 = BagCursor_new(0xb); // TODO: rename field

    FS_UnloadOverlay(0, FS_OVERLAY_ID(OVY_123));

    if ((r4 + ret3) % 0x00000989 != 0) {
        ov124_02260D1C(fsys);
    }

    fsys->unkA8 = sub_02092BB8(0xb);
    fsys->unk108 = FsysUnkSub108_Alloc(0xb);
    fsys->unk114 = GearPhoneRingManager_new(0xb, fsys);
    fsys->unk124 = 0;

    if ((r4 + ret3) % 0x00000fe9 != 0) {
        ov124_02260D1C(fsys);
    }
}

static void *ov124_02260D1C(FieldSystem *fsys) {
    int i;
    int count = 0;
    int size = 0;

    for (i = 0; i < 16; i++) { // todo: replace with constant
        if (PlayerProfile_TestBadgeFlag(Sav2_PlayerData_GetProfileAddr(fsys->savedata), i) == 1) {
            count++;
        }
    }
    if (count == 0) {
        size = 0x00004e20;
    } else {
        size = count * 0x00004e20;
    }
    return AllocFromHeapAtEnd(3, size);
}

static void *ov124_02260D58(void) {
    return AllocFromHeapAtEnd(3, 0x3E8);
}

static void ov124_02260D68(void) {}

static void ov124_02260D6C(void) {}
