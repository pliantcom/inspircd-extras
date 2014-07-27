/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2014 Jansteen <pliantcom@yandex.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"

/* $ModDesc: Provides support for blocking DCC transfers and DCCALLOW */
/* $ModAuthor: Jansteen */
/* $ModAuthorMail: pliantcom@yandex.com */


class CommandDCCBlock : public Command
{
 public:
	CommandDCCBlock(Module* parent) : Command(parent, "DCCALLOW", 0)
	{
		syntax = "";
	}

	CmdResult Handle(const std::vector<std::string> &parameters, User *user)
	{
		/* Display error about DCCALLOW being blocked */
		DisplayError(user);
		return CMD_FAILURE;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_BROADCAST;
	}

	void DisplayError(User* user)
	{
		user->WriteNumeric(998, "%s :DCC not allowed on this server.  No exceptions allowed.", user->nick.c_str());

		LocalUser* localuser = IS_LOCAL(user);
		if (localuser)
			localuser->CommandFloodPenalty += 4000;
	}
};

class ModuleDCCBlock : public Module
{
	CommandDCCBlock cmd;
 public:

        ModuleDCCBlock()
                : cmd(this)
        { }

        void init()
        {
                ServerInstance->Modules->AddService(cmd);
                Implementation eventlist[] = { I_OnUserPreMessage, I_OnUserPreNotice };
                ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
        }

	virtual ModResult OnUserPreMessage(User* user, void* dest, int target_type, std::string &text, char status, CUList &exempt_list)
	{
		return OnUserPreNotice(user, dest, target_type, text, status, exempt_list);
	}

	virtual ModResult OnUserPreNotice(User* user, void* dest, int target_type, std::string &text, char status, CUList &exempt_list)
	{
		if (!IS_LOCAL(user))
			return MOD_RES_PASSTHRU;

                if (target_type == TYPE_USER && text.length() && strncmp(text.c_str(), "\1DCC ", 5) == 0)
		{
			/* This is a DCC request and we want to block it */
			user->WriteNumeric(998, "%s :DCC not allowed on this server.  No exceptions allowed.", user->nick.c_str());
			return MOD_RES_DENY;
		}
		return MOD_RES_PASSTHRU;
	}

	virtual Version GetVersion()
	{
		return Version("Provides support for blocking DCC transfers and DCCALLOW", VF_COMMON );
	}
};

MODULE_INIT(ModuleDCCBlock)
