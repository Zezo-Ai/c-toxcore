/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2025 The TokTok team.
 * Copyright © 2014 Tox project.
 */

/**
 * Slightly better groupchats implementation.
 */
#ifndef C_TOXCORE_TOXCORE_GROUP_H
#define C_TOXCORE_TOXCORE_GROUP_H

#include <stddef.h>
#include <stdint.h>

#include "Messenger.h"
#include "attributes.h"
#include "crypto_core.h"
#include "mem.h"
#include "mono_time.h"
#include "state.h"

typedef enum Groupchat_Type {
    GROUPCHAT_TYPE_TEXT,
    GROUPCHAT_TYPE_AV,
} Groupchat_Type;

typedef void peer_on_join_cb(void *_Nullable object, uint32_t conference_number, uint32_t peer_number);
typedef void peer_on_leave_cb(void *_Nullable object, uint32_t conference_number, void *_Nullable peer_object);
typedef void group_on_delete_cb(void *_Nullable object, uint32_t conference_number);

/** @brief Callback for group invites.
 *
 * data of length is what needs to be passed to `join_groupchat()`.
 */
typedef void g_conference_invite_cb(Messenger *_Nonnull m, uint32_t friend_number, int type, const uint8_t *_Nonnull cookie,
                                    size_t length, void *_Nullable user_data);

/** Callback for group connection. */
typedef void g_conference_connected_cb(Messenger *_Nonnull m, uint32_t conference_number, void *_Nullable user_data);

/** Callback for group messages. */
typedef void g_conference_message_cb(Messenger *_Nonnull m, uint32_t conference_number, uint32_t peer_number, int type,
                                     const uint8_t *_Nonnull message, size_t length, void *_Nullable user_data);

/** Callback for peer nickname changes. */
typedef void peer_name_cb(Messenger *_Nonnull m, uint32_t conference_number, uint32_t peer_number, const uint8_t *_Nonnull name,
                          size_t length, void *_Nullable user_data);

/** Set callback function for peer list changes. */
typedef void peer_list_changed_cb(Messenger *_Nonnull m, uint32_t conference_number, void *_Nullable user_data);

/** @brief Callback for title changes.
 *
 * If peer_number == -1, then author is unknown (e.g. initial joining the group).
 */
typedef void title_cb(Messenger *_Nonnull m, uint32_t conference_number, uint32_t peer_number, const uint8_t *_Nonnull title,
                      size_t length, void *_Nullable user_data);

/** @brief Callback for lossy packets.
 *
 * NOTE: Handler must return 0 if packet is to be relayed, -1 if the packet should not be relayed.
 */
typedef int lossy_packet_cb(void *_Nullable object, uint32_t conference_number, uint32_t peer_number, void *_Nullable peer_object,
                            const uint8_t *_Nonnull packet, uint16_t length);

typedef struct Group_Chats Group_Chats;

const Mono_Time *_Nonnull g_mono_time(const Group_Chats *_Nonnull g_c);

/** Set the callback for group invites. */
void g_callback_group_invite(Group_Chats *_Nonnull g_c, g_conference_invite_cb *_Nonnull function);

/** Set the callback for group connection. */
void g_callback_group_connected(Group_Chats *_Nonnull g_c, g_conference_connected_cb *_Nonnull function);

/** Set the callback for group messages. */
void g_callback_group_message(Group_Chats *_Nonnull g_c, g_conference_message_cb *_Nonnull function);

/** Set callback function for title changes. */
void g_callback_group_title(Group_Chats *_Nonnull g_c, title_cb *_Nonnull function);

/** @brief Set callback function for peer nickname changes.
 *
 * It gets called every time a peer changes their nickname.
 */
void g_callback_peer_name(Group_Chats *_Nonnull g_c, peer_name_cb *_Nonnull function);

/** @brief Set callback function for peer list changes.
 *
 * It gets called every time the name list changes(new peer, deleted peer)
 */
void g_callback_peer_list_changed(Group_Chats *_Nonnull g_c, peer_list_changed_cb *_Nonnull function);

/** @brief Creates a new groupchat and puts it in the chats array.
 *
 * @param rng Random number generator used for generating the group ID.
 * @param type is one of `GROUPCHAT_TYPE_*`
 *
 * @return group number on success.
 * @retval -1 on failure.
 */
int add_groupchat(Group_Chats *_Nonnull g_c, const Random *_Nonnull rng, uint8_t type);

/** @brief Delete a groupchat from the chats array, informing the group first as
 * appropriate.
 *
 * @retval true on success.
 * @retval false if groupnumber is invalid.
 */
bool del_groupchat(Group_Chats *_Nonnull g_c, uint32_t groupnumber, bool leave_permanently);

/**
 * @brief Copy the public key of (frozen, if frozen is true) peernumber who is in
 *   groupnumber to pk.
 *
 * @param pk must be CRYPTO_PUBLIC_KEY_SIZE long.
 *
 * @retval 0 on success
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if peernumber is invalid.
 */
int group_peer_pubkey(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber, uint8_t *_Nonnull pk, bool frozen);

/**
 * @brief Return the size of (frozen, if frozen is true) peernumber's name.
 *
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if peernumber is invalid.
 */
int group_peername_size(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber, bool frozen);

/**
 * @brief Copy the name of (frozen, if frozen is true) peernumber who is in
 *   groupnumber to name.
 *
 * @param  name must be at least MAX_NAME_LENGTH long.
 *
 * @return length of name if success
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if peernumber is invalid.
 */
int group_peername(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber, uint8_t *_Nonnull name, bool frozen);

/**
 * @brief Copy last active timestamp of frozen peernumber who is in groupnumber to
 *   last_active.
 *
 * @retval 0 on success.
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if peernumber is invalid.
 */
int group_frozen_last_active(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber, uint64_t *_Nonnull last_active);

/** @brief Set maximum number of frozen peers.
 *
 * @retval 0 on success.
 * @retval -1 if groupnumber is invalid.
 */
int group_set_max_frozen(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t maxfrozen);

/** @brief invite friendnumber to groupnumber.
 *
 * @retval 0 on success.
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if invite packet failed to send.
 * @retval -3 if we are not connected to the group chat.
 */
int invite_friend(const Group_Chats *_Nonnull g_c, uint32_t friendnumber, uint32_t groupnumber);

/** @brief Join a group (we need to have been invited first).
 *
 * @param expected_type is the groupchat type we expect the chat we are joining
 *   to have.
 *
 * @return group number on success.
 * @retval -1 if data length is invalid.
 * @retval -2 if group is not the expected type.
 * @retval -3 if friendnumber is invalid.
 * @retval -4 if client is already in this group.
 * @retval -5 if group instance failed to initialize.
 * @retval -6 if join packet fails to send.
 */
int join_groupchat(Group_Chats *_Nonnull g_c, uint32_t friendnumber, uint8_t expected_type, const uint8_t *_Nonnull data, uint16_t length);

/** @brief send a group message
 * @retval 0 on success
 * @see send_message_group for error codes.
 */
int group_message_send(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, const uint8_t *_Nonnull message, uint16_t length);

/** @brief send a group action
 * @retval 0 on success
 * @see send_message_group for error codes.
 */
int group_action_send(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, const uint8_t *_Nonnull action, uint16_t length);

/** @brief set the group's title, limited to MAX_NAME_LENGTH.
 * @retval 0 on success
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if title is too long or empty.
 * @retval -3 if packet fails to send.
 */
int group_title_send(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, const uint8_t *_Nonnull title, uint8_t title_len);

/** @brief return the group's title size.
 * @retval -1 of groupnumber is invalid.
 * @retval -2 if title is too long or empty.
 */
int group_title_get_size(const Group_Chats *_Nonnull g_c, uint32_t groupnumber);

/** @brief Get group title from groupnumber and put it in title.
 *
 * Title needs to be a valid memory location with a size of at least MAX_NAME_LENGTH (128) bytes.
 *
 * @return length of copied title if success.
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if title is too long or empty.
 */
int group_title_get(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint8_t *_Nonnull title);

/**
 * @return the number of (frozen, if frozen is true) peers in the group chat on success.
 * @retval -1 if groupnumber is invalid.
 */
int group_number_peers(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, bool frozen);

/**
 * @retval 1 if the peernumber corresponds to ours.
 * @retval 0 if the peernumber is not ours.
 * @retval -1 if groupnumber is invalid.
 * @retval -2 if peernumber is invalid.
 * @retval -3 if we are not connected to the group chat.
 */
int group_peernumber_is_ours(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber);

/** Set handlers for custom lossy packets. */
void group_lossy_packet_registerhandler(Group_Chats *_Nonnull g_c, uint8_t byte, lossy_packet_cb *_Nonnull function);

/** @brief High level function to send custom lossy packets.
 *
 * @retval -1 on failure.
 * @retval 0 on success.
 */
int send_group_lossy_packet(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, const uint8_t *_Nonnull data, uint16_t length);

/**
 * @brief Return the number of chats in the instance m.
 *
 * You should use this to determine how much memory to allocate
 * for copy_chatlist.
 */
uint32_t count_chatlist(const Group_Chats *_Nonnull g_c);

/** @brief Copy a list of valid chat IDs into the array out_list.
 *
 * If out_list is NULL, returns 0.
 * Otherwise, returns the number of elements copied.
 * If the array was too small, the contents
 * of out_list will be truncated to list_size.
 */
uint32_t copy_chatlist(const Group_Chats *_Nonnull g_c, uint32_t *_Nonnull out_list, uint32_t list_size);

/** @brief return the type of groupchat (GROUPCHAT_TYPE_) that groupnumber is.
 *
 * @retval -1 on failure.
 * @return type on success.
 */
int group_get_type(const Group_Chats *_Nonnull g_c, uint32_t groupnumber);

/** @brief Copies the unique id of `group_chat[groupnumber]` into `id`.
 *
 * @retval false on failure.
 * @retval true on success.
 */
bool conference_get_id(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint8_t *_Nonnull id);

int32_t conference_by_id(const Group_Chats *_Nonnull g_c, const uint8_t *_Nonnull id);

/** Send current name (set in messenger) to all online groups. */
void send_name_all_groups(const Group_Chats *_Nonnull g_c);

/** @brief Set the object that is tied to the group chat.
 *
 * @retval 0 on success.
 * @retval -1 on failure
 */
int group_set_object(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, void *_Nullable object);
/** @brief Set the object that is tied to the group peer.
 *
 * @retval 0 on success.
 * @retval -1 on failure
 */
int group_peer_set_object(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber, void *_Nullable object);
/** @brief Return the object tied to the group chat previously set by group_set_object.
 *
 * @retval NULL on failure.
 * @return object on success.
 */
void *_Nullable group_get_object(const Group_Chats *_Nonnull g_c, uint32_t groupnumber);

/** @brief Return the object tied to the group chat peer previously set by group_peer_set_object.
 *
 * @retval NULL on failure.
 * @return object on success.
 */
void *_Nullable group_peer_get_object(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, uint32_t peernumber);

/** @brief Set a function to be called when a new peer joins a group chat.
 *
 * @retval 0 on success.
 * @retval -1 on failure.
 */
int callback_groupchat_peer_new(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, peer_on_join_cb *_Nullable function);
/** @brief Set a function to be called when a peer leaves a group chat.
 *
 * @retval 0 on success.
 * @retval -1 on failure.
 */
int callback_groupchat_peer_delete(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, peer_on_leave_cb *_Nullable function);
/** @brief Set a function to be called when the group chat is deleted.
 *
 * @retval 0 on success.
 * @retval -1 on failure.
 */
int callback_groupchat_delete(const Group_Chats *_Nonnull g_c, uint32_t groupnumber, group_on_delete_cb *_Nullable function);
/** Return size of the conferences data (for saving). */
uint32_t conferences_size(const Group_Chats *_Nonnull g_c);

/** Save the conferences in data (must be allocated memory of size at least `conferences_size()`). */
uint8_t *_Nonnull conferences_save(const Group_Chats *_Nonnull g_c, uint8_t *_Nonnull data);

/**
 * Load a state section.
 *
 * @param data Data to load
 * @param length Length of data
 * @param type Type of section (`STATE_TYPE_*`)
 * @param status Result of loading section is stored here if the section is handled.
 * @return true iff section handled.
 */
bool conferences_load_state_section(Group_Chats *_Nonnull g_c, const uint8_t *_Nonnull data, uint32_t length, uint16_t type, State_Load_Status *_Nonnull status);

/** Create new groupchat instance. */
Group_Chats *_Nullable new_groupchats(const Mono_Time *_Nonnull mono_time, const Memory *_Nonnull mem, Messenger *_Nonnull m);

/** main groupchats loop. */
void do_groupchats(Group_Chats *_Nonnull g_c, void *_Nullable userdata);
/** Free everything related with group chats. */
void kill_groupchats(Group_Chats *_Nullable g_c);
#endif /* C_TOXCORE_TOXCORE_GROUP_H */
