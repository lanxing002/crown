import crown


class FPSCamera:
    def __init__(self, world: crown.World, unit: crown.UnitId):
        self._world = world
        self._unit = unit
        self._move_speed = 20
        self._rotation_speed = 0.14

    @property
    def unit(self):
        return self._unit

    def camera(self):
        return self._world.camera_instance(self._unit)

    def upate(self, dt, dx, dy):
        trans_ins = self._world.scene_graph.instance(self._unit)
        camera_pose = self._world.scene_graph.local_pose(trans_ins)
        pos = camera_pose.translation
        view_dir = camera_pose.z
        right_dir = camera_pose.x

        if dx > 0 and dy > 0:
            rot_delta = self._rotation_speed * dt
            rot_around_world_up = crown.Quaternion(crown.Vector3(0, 1, 0), dx * rot_delta)
            rot_around_camera_right = crown.Quaternion(camera_pose.x, dy * rot_delta)
            rotation = rot_around_world_up * rot_around_camera_right

            old_rot = crown.math.from_quaternion(camera_pose.rotation)
            delta_rot = crown.math.from_quaternion(rotation)
            new_rot = old_rot * delta_rot
            new_rot.translation = pos

            self._world.scene_graph.set_local_pose(trans_ins, new_rot)

        w, s, a, d = [crown.keyboard.button_id(x) for x in ['w', 's', 'a', 'd']]
        wkey = crown.keyboard.pressed(w) or crown.keyboard.released(w)
        skey = crown.keyboard.pressed(s) or crown.keyboard.released(s)
        akey = crown.keyboard.pressed(a) or crown.keyboard.released(a)
        dkey = crown.keyboard.pressed(d) or crown.keyboard.released(d)
        translation_speed = self._move_speed * dt
        if wkey:
            pos += view_dir * translation_speed * dt

        if skey:
            pos -= view_dir * translation_speed * dt

        if akey:
            pos -= right_dir * translation_speed * dt

        if dkey:
            pos += right_dir * translation_speed * dt

        self._world.scene_graph.set_local_position(trans_ins, pos)


