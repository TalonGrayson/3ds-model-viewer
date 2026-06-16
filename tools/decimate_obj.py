"""
decimate_obj.py — Blender headless script to decimate an OBJ and re-export it.

Usage (run via Blender's Python interpreter):
  /Applications/Blender.app/Contents/MacOS/Blender --background --python tools/decimate_obj.py -- input.obj output.obj [ratio]

  ratio: decimate ratio in (0,1] — lower = fewer faces (default: 0.05)
"""

import bpy
import sys
import os

argv = sys.argv
argv = argv[argv.index("--") + 1:]  # everything after --

input_obj  = argv[0]
output_obj = argv[1]
ratio      = float(argv[2]) if len(argv) > 2 else 0.05

# Clear the default scene
bpy.ops.wm.read_factory_settings(use_empty=True)

# Import
bpy.ops.wm.obj_import(filepath=input_obj)

# Decimate every mesh object
for obj in bpy.context.scene.objects:
    if obj.type != 'MESH':
        continue
    bpy.context.view_layer.objects.active = obj
    mod = obj.modifiers.new(name='Decimate', type='DECIMATE')
    mod.ratio = ratio
    bpy.ops.object.modifier_apply(modifier=mod.name)

# Export — triangulate on the way out, include normals
bpy.ops.wm.obj_export(
    filepath=output_obj,
    export_triangulated_mesh=True,
    export_normals=True,
    export_uv=False,
    export_materials=False,
)

tri_count = sum(
    len(obj.data.polygons)
    for obj in bpy.context.scene.objects
    if obj.type == 'MESH'
)
print(f"[decimate_obj] {tri_count} triangles → {output_obj}")
