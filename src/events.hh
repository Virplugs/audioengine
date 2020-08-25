#pragma once

#include <functional>
#include <napi.h>
#include <utility>

void sendEventNonBlocking(const char *eventName, std::function<Napi::Value(Napi::Env)> eventFn);
void sendEventNonBlocking(const char *eventName);
void sendEventBlocking(const char *eventName, std::function<Napi::Value(Napi::Env)> eventFn);
void sendEventBlocking(const char *eventName);
