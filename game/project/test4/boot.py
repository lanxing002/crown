import crown
import math


class Game:

    def __int__(self):
        self.world: crown.World = None
        self.camera: crown.CameraInstance = None
        self.camera_unit: crown.UnitId = None
        self.players: list[crown.UnitId] = None
        self.avatar: crown.UnitId = None


game = Game()
device = crown.Device.g_device


# world: crown.World = None
# camera_unit: crown.UnitId = None


def init():
    # Create world and camera
    world = device.create_world()
    camera_unit = world.spawn_unit("core/units/camera")
    scene_graph = world.scene_graph

    # game load level
    camera = world.camera_instance(camera_unit)
    world.camera_set_orthographic_size(camera, 540 / 2 / 32)
    world.camera_set_projection_type(camera, crown.ProjectionType.ORTHOGRAPHIC)
    camera_trans = scene_graph.instance(camera_unit)
    scene_graph.set_local_position(camera_trans, crown.Vector3(0, 8, 0))
    scene_graph.set_local_rotation(camera_trans, crown.math.from_axis_angle(crown.math.vec3_right, 90 * (math.pi / 180.0)))

    player1 = world.spawn_unit("units/soldier", crown.Vector3(-2, 0, 0))
    player2 = world.spawn_unit("units/princess", crown.Vector3(-8, 0, 0))

    global game
    game.avatar = player1
    game.players = [player1, player2]
    game.world = world
    game.camera = camera
    game.camera_unit = camera_unit
    print('----> init world done <-----')


cnt = 0


def update(dt):
    global game, cnt
    game.world.update(dt)
    asm = game.world.animation_state_machine
    asm.trigger(asm.instance(game.avatar), 'run')
    cnt += 1
    if (cnt // 40) % 2 == 0:
        asm.trigger(asm.instance(game.avatar), 'idle')
    # if crown.keyboard.released(crown.keyboard.button_id("escape")):
    # device.quit()
    # print('escape key pushed ... ')


def render(dt):
    global game, device
    device.render(game.world, game.camera_unit)
    pass


def shutdown():
    pass
