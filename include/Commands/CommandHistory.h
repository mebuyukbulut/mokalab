#pragma once
#include <memory>
#include <vector>
#include "Commands/ICommand.h"

class CommandHistory {
public:
    static void executeCommand(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
        _undoStack.push_back(std::move(cmd));
        _redoStack.clear();
    }

    static void pushExecutedCommand(std::unique_ptr<ICommand> cmd) {
        _undoStack.push_back(std::move(cmd));
        _redoStack.clear();
    }

    static void undo() {
        if (_undoStack.empty()) return;
        auto cmd = std::move(_undoStack.back());
        _undoStack.pop_back();
        
        cmd->undo();
        _redoStack.push_back(std::move(cmd));
    }

    static void redo() {
        if (_redoStack.empty()) return;
        auto cmd = std::move(_redoStack.back());
        _redoStack.pop_back();
        
        cmd->execute();
        _undoStack.push_back(std::move(cmd));
    }

private:
    inline static std::vector<std::unique_ptr<ICommand>> _undoStack;
    inline static std::vector<std::unique_ptr<ICommand>> _redoStack;
};