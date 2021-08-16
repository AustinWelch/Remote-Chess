#pragma once

namespace RemoteChess {
    struct Friend {
        int id;
        char name[9];

        bool operator==(const Friend& rhs) const {
        	return id == rhs.id && strcmp(name, rhs.name) == 0;
        }
    };
}
