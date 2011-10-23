/* See LICENSE file for license and copyright information */

#ifndef GIRARA_COMPLETION_H
#define GIRARA_COMPLETION_H

#include "types.h"

/**
 * Structure of a completion element
 */
struct girara_completion_element_s
{
  char *value; /**> Name of the completion element */
  char *description; /**> Description of the completion element */
  struct girara_completion_element_s *next; /**> Next completion element (linked list) */
};

/**
 * Structure of a completion group
 */
struct girara_completion_group_s
{
  char *value; /**> Name of the completion element */
  girara_completion_element_t *elements; /**> Elements of the completion group */
  struct girara_completion_group_s *next; /**> Next group (linked list) */
};

/**
 * Structure of a completion object
 */
struct girara_completion_s
{
  girara_completion_group_t *groups; /**> Containing completion groups */
};

/**
 * Creates an girara completion object
 *
 * @return Completion object
 * @return NULL An error occured
 */
girara_completion_t* girara_completion_init();

/**
 * Creates an girara completion group
 *
 * @return Completion object
 * @return NULL An error occured
 */
girara_completion_group_t* girara_completion_group_create(girara_session_t* session, char* name);

/**
 * Frees a completion group
 *
 * @param group The group
 */
void girara_completion_group_free(girara_completion_group_t* group);

/**
 * Adds an group to a completion object
 *
 * @param completion The completion object
 * @param group The completion group
 */
void girara_completion_add_group(girara_completion_t* completion, girara_completion_group_t* group);

/**
 * Frees an completion and all of its groups and elements
 *
 * @param completion The completion
 */
void girara_completion_free(girara_completion_t* completion);

/**
 * Adds an element to a completion group
 *
 * @param group The completion group
 * @param value Value of the entry
 * @param description Description of the entry
 */
void girara_completion_group_add_element(girara_completion_group_t* group, char* value, char* description);

#endif
