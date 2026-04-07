<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\SubnetValidator;

/**
 * Defines a model that interacts with the REST API access list entries.
 */
class RESTAPIAccessListEntry extends Model {
    public StringField $type;
    public IntegerField $weight;
    public StringField $network;
    public ForeignModelField $users;
    public ForeignModelField $sched;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Gather information about current API settings
        $api_pkg_id = RESTAPISettings::get_pkg_id();

        # Set Model attributes
        $this->config_path = "installedpackages/package/$api_pkg_id/conf/access_list/item";
        $this->many = true;
        $this->many_minimum = 1;
        $this->verbose_name_plural = 'REST API Access List Entries';
        $this->sort_order = SORT_ASC;
        $this->sort_by = ['weight'];

        # Define model fields
        $this->type = new StringField(
            default: 'allow',
            choices: ['allow', 'deny'],
            verbose_name: 'Type',
            help_text: 'The type of access this entry provides. "allow" entries permit access to the REST API ' .
                'from the specified networks. "deny" entries block access to the REST API from the specified networks.',
        );
        $this->weight = new IntegerField(
            default: 1,
            minimum: 0,
            maximum: 100,
            verbose_name: 'Weight',
            help_text: 'The weight of this entry. Entries with lower weights are evaluated first. If multiple ' .
                'entries match a request, the entry with the lowest weight will be applied.',
        );
        $this->network = new StringField(
            required: true,
            validators: [new SubnetValidator()],
            verbose_name: 'Network',
            help_text: 'The network (in CIDR notation) that this entry applies to. Clients interacting with the ' .
                'REST API from this network will be affected by this entry.',
        );
        $this->users = new ForeignModelField(
            model_name: 'User',
            model_field: 'name',
            default: [],
            choices_callable: 'get_user_choices',
            allow_empty: true,
            many: true,
            many_minimum: 0,
            verbose_name: 'Users',
            help_text: 'The users that this entry applies to. Only users in this list will be affected by this ' .
                'entry. Leave empty if this entry should apply to all users.',
        );
        $this->sched = new ForeignModelField(
            model_name: 'FirewallSchedule',
            model_field: 'name',
            default: '',
            choices_callable: 'get_schedule_choices',
            allow_empty: true,
            verbose_name: 'schedule',
            help_text: 'The firewall schedule that this entry will use. This entry will only be active during the 
                times specified in the schedule. Leave empty to apply this entry at all times.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 256,
            verbose_name: 'description',
            help_text: 'A description of this access list entry. This field is optional.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Returns a list of user names to be used as choices for the users field
     * @return array The list of user names.
     */
    public static function get_user_choices(): array {
        $users = [];
        foreach (User::read_all()->model_objects as $user) {
            $users[$user->name->value] = ($user->descr->value ?? '-') . " ({$user->name->value})";
        }
        return $users;
    }

    /**
     * Returns a list of schedule names to be used as choices for the schedule field
     * @return array The list of schedule names.
     */
    public static function get_schedule_choices(): array {
        $schedules = ['' => 'None'];
        foreach (FirewallSchedule::read_all()->model_objects as $schedule) {
            $schedules[$schedule->name->value] = $schedule->name->value;
        }
        return $schedules;
    }

    /**
     * Checks if a given IP address is within the network range of this access list entry.
     * @param string $ip The IP address to check.
     * @return bool true if the IP address is within the network range, false otherwise.
     */
    public function is_ip_match(string $ip): bool {
        return ip_in_subnet($ip, $this->network->value);
    }

    /**
     * Checks if a given user is in the list of users for this access list entry.
     * @param string $username The username to check.
     * @return bool true if the user is in the list, false otherwise.
     */
    public function is_user_match(string $username): bool {
        return empty($this->users->value) or in_array($username, $this->users->value);
    }

    /**
     * Checks if the assigned schedule is active at the current time.
     * @return bool true if the schedule is active, false otherwise.
     */
    public function is_schedule_active(): bool {
        # If no schedule is assigned, assume it is always active
        if (empty($this->sched->value)) {
            return true;
        }

        # If the assigned schedule no longer exists, log the event and deny access
        if (!$this->sched->get_related_model()) {
            $this->log(
                level: LOG_ERR,
                message: "The schedule '{$this->sched->value}' assigned to REST API access list entry " .
                    "'$this->id' no longer exists.",
            );
            return false;
        }

        return $this->sched->get_related_model()->active->value;
    }

    /**
     * Checks if a given IP address and username are allowed by any active access list entry.
     * @param string $ip The IP address to check.
     * @param string $username The username to check.
     * @return bool true if the IP address and username are allowed by an active ACL entry, false otherwise.
     */
    public static function is_allowed_by_acl(string $ip, string $username): bool {
        # Check each access list entry for a match
        foreach (RESTAPIAccessListEntry::read_all()->model_objects as $acl) {
            # If the IP and username match, and the schedule is active, return the determination based on ACL type
            if ($acl->is_ip_match($ip) and $acl->is_user_match($username) and $acl->is_schedule_active()) {
                # Return true if the matched ACL's type is 'allow', otherwise false (deny)
                return $acl->type->value === 'allow';
            }
        }

        # Implicitly deny access if no ACL entry matches
        return false;
    }
}
