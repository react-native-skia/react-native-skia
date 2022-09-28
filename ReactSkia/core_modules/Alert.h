/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

namespace facebook {
namespace react {

class Alert {
public:

    Alert(std::string title,std::string message) {
        title_ = title;
        message_ = message;
    };
    ~Alert() = default;

    // Getter
    std::string getTitle(){return title_;}
    std::string getMessage(){return message_;}
private:

    std::string title_;
    std::string message_;
};
} // react
} // facebook
