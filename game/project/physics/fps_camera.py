import crown

class FPSCamera:
    def __init__(self, world: crown.World, unit: crown.UnitId):
        self._world = world
        self._unit = unit
        self._move_speed = 40
        self._rotation_speed = 0.44
        self._keys = [crown.keyboard.button_id(x) for x in ['w', 's', 'a', 'd']]
        self._keys_pressed = [False, False, False, False]  # w s a d

    @property
    def unit(self):
        return self._unit

    def camera(self):
        return self._world.camera_instance(self._unit)

    def update(self, dt, dx, dy):
        trans_ins = self._world.scene_graph.instance(self._unit)
        camera_pose = self._world.scene_graph.local_pose(trans_ins)
        pos = camera_pose.translation
        view_dir = crown.Vector3(camera_pose.z)
        right_dir = crown.Vector3(camera_pose.x)

        if dx != 0 and dy != 0:
            rot_delta = self._rotation_speed * dt
            rot_around_world_up = crown.Quaternion(crown.Vector3(0, 1, 0), dx * rot_delta)
            rot_around_camera_right = crown.Quaternion(crown.Vector3(camera_pose.x), dy * rot_delta)
            rotation = rot_around_world_up * rot_around_camera_right

            old_rot = crown.math.from_quaternion(camera_pose.rotation)
            delta_rot = crown.math.from_quaternion(rotation)
            new_rot = old_rot * delta_rot
            new_rot.translation = pos

            self._world.scene_graph.set_local_pose(trans_ins, new_rot)

        for i in range(len(self._keys)):
            if crown.keyboard.pressed(self._keys[i]):
                self._keys_pressed[i] = True

        for i in range(len(self._keys)):
            if crown.keyboard.released(self._keys[i]):
                self._keys_pressed[i] = False

        translation_speed = self._move_speed * dt
        if self._keys_pressed[0]:
            pos += view_dir * translation_speed

        if self._keys_pressed[1]:
            pos -= view_dir * translation_speed

        if self._keys_pressed[2]:
            pos -= right_dir * translation_speed

        if self._keys_pressed[3]:
            pos += right_dir * translation_speed

        self._world.scene_graph.set_local_position(trans_ins, pos)


