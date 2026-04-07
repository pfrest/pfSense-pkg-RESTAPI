<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\ObjectField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\NotFoundError;

/**
 * Defines a Model for calculating all possible choices for a given model and field. This is used as a definitive
 * source for determine what choices are available to a field, whether it's static choices, dynamic choices from
 * a choices callable, or a foreign model reference via a ForeignModelField
 */
class Enum extends Model {
    public StringField $model_name;
    public StringField $model_field;
    public ObjectField $choices;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->many = false;
        $this->internal_callable = 'void_callable';

        # Define model Fields
        $this->model_name = new StringField(
            required: true,
            verbose_name: 'Model Name',
            help_text: "The name of the model whose field's choices are being requested.",
        );
        $this->model_field = new StringField(
            required: true,
            verbose_name: 'Model Field',
            help_text: 'The name of the field whose choices are being requested.',
        );
        $this->choices = new ObjectField(
            default: null,
            read_only: true,
            verbose_name: 'Choices',
            help_text: 'The available choices for the specified model and field. The key represents the internal ' .
                'name of the choice, and the value is the verbose, human-friendly name of the choice.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * A void callable to mock an internal callable. This Model does not support reads.
     */
    public function void_callable(): array {
        return [];
    }

    /**
     * Ensures the requested `model_name` is actually a valid model.
     *
     * @param string $model_name The $model_name value to be validated.
     * @returns string The validated $model_name to be set.
     * @throws NotFoundError if the specific model does not exist.
     */
    public function validate_model_name(string $model_name): string {
        if (!in_array($model_name, Model::get_all_model_classes(shortnames: true))) {
            throw new NotFoundError(
                message: "Model '$model_name' does not exist.",
                response_id: 'ENUM_MODEL_DOES_NOT_EXIST',
            );
        }

        return $model_name;
    }

    /**
     * Ensure the selected `model_field` is actually a field within the requested model.
     *
     * @param string $model_field The name of the field to validate.
     * @return string The validated $field_name value to be set.
     * @throws NotFoundError if the specified field does not exist on the specified model.
     */
    public function validate_model_field(string $model_field): string {
        $model_fqn = "\\RESTAPI\\Models\\{$this->model_name->value}";
        $model = new $model_fqn(skip_init: true);

        if (!in_array($model_field, $model->get_fields())) {
            throw new NotFoundError(
                message: "Field '$model_field' does not exist on model '{$this->model_name->value}'.",
                response_id: 'ENUM_MODEL_FIELD_DOES_NOT_EXIST',
            );
        }

        return $model_field;
    }

    public function _create(): void {
        $model_field = $this->model_field->value;
        $model_fqn = "\\RESTAPI\\Models\\{$this->model_name->value}";
        $model = new $model_fqn(skip_init: true);

        # If this model has static choices, set all available choices
        if ($model->$model_field->choices) {
            $this->choices->value = $model->$model_field->verbose_choices;
        }

        # If this model has a choices callable, resolve the choices
        elseif ($model->$model_field->choices_callable) {
            $model->$model_field->set_choices_from_callable();
            $this->choices->value = $model->$model_field->verbose_choices;
        }

        # If this model is a ForeignModelField, resolve the choices from the foreign model
        elseif ($model->$model_field instanceof ForeignModelField) {
            $f_model_field = $model->$model_field->model_field;
            $in_scope_modelsets = $model->$model_field->get_in_scope_models();
            foreach ($in_scope_modelsets as $in_scope_modelset) {
                foreach ($in_scope_modelset->model_objects as $model_object) {
                    $this->choices->value[$model_object->$f_model_field->value] = $model_object->$f_model_field->value;
                }
            }
        } else {
            throw new ConflictError(
                message: "Field '$model_field' does not support choices.",
                response_id: 'ENUM_MODEL_FIELD_DOES_NOT_SUPPORT_CHOICES',
            );
        }
    }
}
