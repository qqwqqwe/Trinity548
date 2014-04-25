/*
 * Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "zlib.h"
#include "AddonHandler.h"
#include "AddonMgr.h"
#include "DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"

AddonHandler::AddonHandler() { }

AddonHandler::~AddonHandler() { }

// This is not used anywhere, SendAddonsInfo() in WorldSession.cpp is.
/*bool AddonHandler::BuildAddonPacket(WorldPacket* source, WorldPacket* target)
{
    ByteBuffer AddOnPacked;
    uLongf AddonRealSize;
    uint32 CurrentPosition;
    uint32 TempValue;

    // broken addon packet, can't be received from real client
    if (source->rpos() + 4 > source->size())
        return false;

    *source >> TempValue;                                   // get real size of the packed structure

    // empty addon packet, nothing process, can't be received from real client
    if (!TempValue)
        return false;

    AddonRealSize = TempValue;                              // temp value because ZLIB only excepts uLongf

    CurrentPosition = source->rpos();                       // get the position of the pointer in the structure

    AddOnPacked.resize(AddonRealSize);                      // resize target for zlib action

    if (uncompress(AddOnPacked.contents(), &AddonRealSize, source->contents() + CurrentPosition, source->size() - CurrentPosition) == Z_OK)
    {
        // Variables to use in second part.
        uint8 HasURLString = 0;
        uint8 HasPublicKeyOrCRC = 0;
        uint8 HasPublicKey = 0;
        uint8 AddonState = 1;
        uint32 CRC = 0;

        // Now build the packet.
        target->Initialize(SMSG_ADDON_INFO);

        uint32 addonsCount;
        AddOnPacked >> addonsCount;                         // Read Addons count.

        target->WriteBits(addonsCount, 23);

        for (uint32 i = 0; i < addonsCount; ++i)
        {
            std::string addonName;
            uint8 enabled; // Or UsePublicKeyOrCRC.
            uint32 crc, urlFile;

            // check next addon data format correctness
            if (AddOnPacked.rpos()+1 > AddOnPacked.size())
                return false;

            AddOnPacked >> addonName;

            // recheck next addon data format correctness
            if (AddOnPacked.rpos()+1+4+4 > AddOnPacked.size())
                return false;

            AddOnPacked >> enabled >> crc >> urlFile;

            TC_LOG_DEBUG("network", "ADDON: Name: %s, Enabled: 0x%x, CRC: 0x%x, Unknown2: 0x%x", addonName.c_str(), enabled, crc, urlFile);

            uint8 hasURLString = ((!enabled && urlFile) ? 1 : 0);
            HasURLString = hasURLString; // Second part variable.

            uint8 UsePublicKeyOrCRC = (enabled ? 1 : 0);
            HasPublicKeyOrCRC = UsePublicKeyOrCRC; // Second part variable.

            uint8 usePublicKey = (crc != STANDARD_ADDON_CRC);          // If addon doesn't have a Standard addon CRC.
            HasPublicKey = usePublicKey; // Second part variable.

            uint8 state = (enabled ? 2 : 1);
            AddonState = state; // Second part variable.

            CRC = crc;

			target->WriteBit(HasPublicKeyOrCRC);
			target->WriteBit(HasURLString);
			target->WriteBit(HasPublicKey);

            if (hasURLString)
                target->WriteBits(0, 8); // URL String size
		}

        AddonMgr::BannedAddonList const* bannedAddons = AddonMgr::GetBannedAddons();
        target->WriteBits(uint32(bannedAddons->size()), 18);

        for (uint32 i = 0; i < addonsCount; ++i)
		{
            if (HasPublicKey)
            {
                uint8 addonPublicKey[256] =
                {
                    0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA, 0x0E, 0x5D, 0x54,
                    0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84, 0x93, 0x45, 0x75,
                    0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40, 0x85, 0x59, 0x34,
                    0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B, 0x95, 0x28, 0xA8,
                    0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A, 0x1A, 0x2C, 0xC8,
                    0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50, 0x37, 0x8F, 0x5A,
                    0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34, 0xC6, 0xC2, 0xC3,
                    0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57, 0xED, 0x46, 0xB9,
                    0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77, 0xCF, 0xAD, 0x22,
                    0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18, 0xF2, 0x92, 0x7E,
                    0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85, 0x9C, 0xCB, 0xFB,
                    0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85, 0xBA, 0xA8, 0xF7,
                    0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7, 0xCE, 0x36, 0xF0,
                    0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3, 0xF5, 0xE0, 0xA6,
                    0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE, 0x79, 0xC3, 0x9A,
                    0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E, 0xC7, 0x89, 0xD2
                };

                uint8 pubKeyOrder[256] = 
                {
                    0x7A, 0xEE, 0x14, 0xB2, 0x6B, 0x4F, 0xDF, 0x04, 0x91, 0x0E, 0x74, 0x58, 0x38, 0xF3, 0x1A, 0xB3,
                    0xE9, 0xD8, 0x51, 0x53, 0x19, 0xF6, 0x08, 0x79, 0x44, 0xED, 0x6A, 0x09, 0x7E, 0xC5, 0xAE, 0x65,
                    0x7F, 0x5F, 0xD7, 0x0F, 0x07, 0x2B, 0x39, 0xE6, 0x2F, 0x3E, 0xC8, 0xA5, 0x81, 0xB6, 0x3A, 0x1D,
                    0x61, 0x06, 0x67, 0x57, 0x92, 0xBA, 0x4A, 0xE5, 0x75, 0x7C, 0xB9, 0x94, 0x2A, 0xEF, 0xD4, 0xF2,
                    0xB7, 0x24, 0xD9, 0xA6, 0xE8, 0x5E, 0xCD, 0x43, 0xDC, 0x2D, 0x05, 0xC6, 0x70, 0x0B, 0x46, 0x34,
                    0xF0, 0x1F, 0xC7, 0x0D, 0x72, 0x2C, 0x4B, 0x1C, 0xE0, 0x9B, 0xE1, 0xC0, 0xCC, 0x98, 0x63, 0xF7,
                    0x27, 0x25, 0xD5, 0x4C, 0x71, 0x02, 0x97, 0xB5, 0xAF, 0x54, 0xFC, 0x00, 0x2E, 0x64, 0xAA, 0xF1,
                    0x88, 0x18, 0xFB, 0x50, 0x03, 0x52, 0x20, 0x86, 0xB8, 0x68, 0x4E, 0x87, 0xBC, 0xA2, 0x13, 0x0C,
                    0xEC, 0xA8, 0xBB, 0x8B, 0x35, 0x42, 0x1E, 0xCB, 0x90, 0x3F, 0xFA, 0xFE, 0x1B, 0x56, 0x85, 0xA7,
                    0x84, 0xDD, 0x30, 0xA0, 0x22, 0x77, 0xA9, 0xF9, 0xE4, 0x73, 0x21, 0xC1, 0xBD, 0xAC, 0xBE, 0xCE,
                    0x9E, 0x6E, 0xD0, 0x16, 0xF4, 0x26, 0x3D, 0xC9, 0xF5, 0x76, 0x45, 0x11, 0x9D, 0x3C, 0x9F, 0x48,
                    0xBF, 0x32, 0x6C, 0x66, 0x9A, 0xDA, 0x17, 0x60, 0x83, 0xB1, 0x80, 0x5C, 0x8A, 0xAB, 0xDE, 0xC4,
                    0x5B, 0x23, 0xCF, 0xD3, 0x62, 0xB4, 0x8E, 0xF8, 0x59, 0x36, 0xA1, 0x8D, 0xE7, 0x0A, 0x9C, 0x78,
                    0x7D, 0xFD, 0x29, 0x3B, 0x47, 0x69, 0x82, 0x15, 0x5D, 0x6F, 0x55, 0x49, 0xEA, 0x93, 0xAD, 0x28,
                    0xDB, 0x89, 0x95, 0x40, 0xEB, 0xB0, 0x33, 0xD2, 0x4D, 0xD6, 0x8F, 0x12, 0x31, 0xA3, 0x8C, 0xE2,
                    0x01, 0x10, 0x96, 0x6D, 0x37, 0xE3, 0xA4, 0xD1, 0x41, 0x99, 0xCA, 0xC3, 0xC2, 0x7B, 0x5A, 0xFF
                };

                size_t pos = target->wpos();
                for (int i = 0; i < 256; i++)
                    *target << uint8(0);

                for (int i = 0; i < 256; i++)
                    target->put(pos + pubKeyOrder[i], addonPublicKey[i]); // Assign Public Keys.
            }

            if (HasPublicKeyOrCRC)
            {
                *target << uint32(CRC);
                *target << uint8(HasPublicKeyOrCRC); // IsEnabled.
            }

            *target << uint8(AddonState);

            if (HasURLString)
                target->WriteString(""); // URL String, 256 (null terminated?).
        }

        uint32 currentTime;
        AddOnPacked >> currentTime;

        for (AddonMgr::BannedAddonList::const_iterator itr = bannedAddons->begin(); itr != bannedAddons->end(); ++itr)
        {
            for (int32 i = 0; i < 8; i++)
                *target << uint32(0);

            *target << uint32(itr->Id);
            *target << uint32(itr->Timestamp);
            *target << uint32(1);  // IsBanned
        }

        if (AddOnPacked.rpos() != AddOnPacked.size())
            TC_LOG_DEBUG("network", "packet under read!");
    }
    else
    {
        TC_LOG_ERROR("network", "Addon packet uncompress error :(");
        return false;
    }

    return true;
}*/
