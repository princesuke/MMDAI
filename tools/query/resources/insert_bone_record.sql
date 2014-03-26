insert into mmq_bones (
  `index`,
  `parent_model_id`,
  `name_ja`,
  `name_en`,
  `parent_bone_id`,
  `destination_bone_id`,
  `is_movable`,
  `is_rotateable`,
  `is_visible`,
  `is_interactive`,
  `is_inherent_translation_enabled`,
  `is_inherent_orientation_enabled`,
  `has_inverse_kinematics`,
  `has_fixed_axes`,
  `has_local_axes`
) values (
  :index,
  :parent_model,
  :name_ja,
  :name_en,
  :parent_bone,
  :destination_bone,
  :is_movable,
  :is_rotateable,
  :is_visible,
  :is_interactive,
  :is_inherent_translation_enabled,
  :is_inherent_orientation_enabled,
  :has_inverse_kinematics,
  :has_fixed_axes,
  :has_local_axes
);