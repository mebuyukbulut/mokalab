#pragma once


class ICommand
{
public:
    ~ICommand() = default;
    virtual void execute() = 0; 
    virtual void undo() = 0; 
};
