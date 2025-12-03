#pragma once

namespace qKratos::Error {

enum ErrorCode : int {
    InvalidParams    = 1000,
    UserNameEmpty    = 1001,
    PasswordTooShort = 1002,
    EmailInvalid     = 1003,
    UserIdEmpty      = 1004,
    UserIdInvalid    = 1006,
    UserNotFound     = 1005,
    Unauthorized     = 1007,
    Forbidden =1008,
    UnknownError     = 9999
};

} // namespace
