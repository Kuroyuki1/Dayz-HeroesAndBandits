
class HeroesAndBanditsZone
{
	int Index; //0 top level zones, 1 sub zone, 2 sub zone of a sub zone etc.
	int ZoneID;
    string Name;
	float X;
	float Z;
	float Radius;
	bool ShowWarningMsg;
	string WarningMessage;
	bool ShowWelcomeMsg;
	string WelcomeMessage;
	string WelcomeIcon;
	int WelcomeMessageColor;
	float KillRadius;
    float MinHumanity;
    float MaxHumanity;
	bool OverrideSafeZone;
	bool GodModPlayers;
	bool PerventActions;
	ref array< ref habZoneAffinity > Affinities = new ref array< ref habZoneAffinity >;
	ref array< ref HeroesAndBanditsGuard > Guards = new ref array< ref HeroesAndBanditsGuard >;
	ref array< ref HeroesAndBanditsZone > SubZones = new ref array< ref HeroesAndBanditsZone >;

	void Init(habZone zoneToLoad, int zoneID, int index = 0){
		Index = index;
		ZoneID = zoneID;
		Name = zoneToLoad.Name;
		X = zoneToLoad.X;
		Z = zoneToLoad.Z;
		MinHumanity = zoneToLoad.MinHumanity;
		MaxHumanity = zoneToLoad.MaxHumanity;
		Radius = zoneToLoad.WarningRadius;
		KillRadius = zoneToLoad.KillRadius;
		WelcomeMessageColor = zoneToLoad.getWelcomeMessageColor();
		ShowWelcomeMsg = zoneToLoad.ShowWelcomeMsg;
		WelcomeMessage = zoneToLoad.WelcomeMessage;
		WelcomeIcon = zoneToLoad.WelcomeIcon;
		ShowWarningMsg = zoneToLoad.ShowWarningMsg;
		WarningMessage = zoneToLoad.WarningMessage;
		OverrideSafeZone = zoneToLoad.OverrideSafeZone;
		GodModPlayers = zoneToLoad.GodModPlayers;
		PerventActions = zoneToLoad.PerventActions;
		Affinities = zoneToLoad.Affinities;
		if (zoneToLoad.Guards){
			for ( int j = 0; j < zoneToLoad.Guards.Count(); j++ )
			{	
				float guardX = zoneToLoad.Guards.Get(j).X;
				float guardY = zoneToLoad.Guards.Get(j).Y; 
				float guardZ = zoneToLoad.Guards.Get(j).Z; 
				float orientation = zoneToLoad.Guards.Get(j).Orientation; 
				string skin = zoneToLoad.Guards.Get(j).Skin; 
				string weaponInHands = zoneToLoad.Guards.Get(j).WeaponInHands; 
				string weaponInHandsMag = zoneToLoad.Guards.Get(j).WeaponInHandsMag;
				TStringArray weaponInHandsAttachments = zoneToLoad.Guards.Get(j).WeaponInHandsAttachments; 
				TStringArray guardGear = zoneToLoad.Guards.Get(j).GuardGear;
				Guards.Insert(new ref HeroesAndBanditsGuard(guardX, guardY, guardZ, orientation, skin, weaponInHands, weaponInHandsMag, weaponInHandsAttachments, guardGear));
				Guards.Get(j).Spawn();
				//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLaterByName(Guards.Get(j), "ReloadWeapon", 60000, false); //Reload gun 3 minutes after server start
				//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLaterByName(Guards.Get(j), "ReloadWeaponTest", 440000, false); //Reload gun 3 minutes after server start	
			}
		}
		if (zoneToLoad.SubZones) //Check if any Sub Zones are defined before loading
		{
			for ( int i = 0; i < zoneToLoad.SubZones.Count(); i++ )
			{
		    	habPrint("Loading Sub Zone " + zoneToLoad.SubZones.Get(i).Name , "Verbose");
				string name = zoneToLoad.SubZones.Get(i).Name;
				int x = zoneToLoad.SubZones.Get(i).X;
				int z = zoneToLoad.SubZones.Get(i).Z;
				SubZones.Insert(new ref HeroesAndBanditsZone(name, x, z));
				SubZones.Get(i).Init(zoneToLoad.SubZones.Get(i), i + 1, Index + 1);
			}
		}
		
	}
		
	void CheckPlayer(PlayerBase inPlayer){
		PlayerBase player = PlayerBase.Cast(inPlayer);
		HeroesAndBanditsPlayer pdata = GetHeroesAndBandits().GetPlayer(player.GetIdentity().GetPlainId());
		if ( !player )
		{
			return;
		}
			
		if (player.IsAlive())				
		{
			
			/*habPrint("Checking if Player: " + player.GetIdentity().GetName() + " ("+player.GetIdentity().GetPlainId()+") is in Zone " + Name, "Debug");	
			if ( GetHeroesAndBanditsSettings().DebugLogs ){
				player.m_HeroesAndBandits_InZones.Debug();
			}*/
			if (validPlayer(pdata) && vector.Distance(player.GetPosition(), getVector()) <= Radius && !player.habIsInZone(ZoneID, Index)){
				habPrint("Player: " + player.GetIdentity().GetName() + " ("+player.GetIdentity().GetPlainId()+") Entered: " + Name, "Verbose");
				player.enteredZone(ZoneID, Index);
				if ( GetHeroesAndBanditsSettings().DebugLogs ){
					player.m_HeroesAndBandits_InZones.Debug();
				}
				if ( ShowWelcomeMsg )
				{
					GetHeroesAndBandits().WelcomePlayer(Name, WelcomeMessage, WelcomeIcon, player.GetIdentity().GetPlainId(), WelcomeMessageColor);
				}
				if ( GodModPlayers && validPlayer(pdata))
				{
					player.SetAllowDamage(false);
				}
			}
			else if (!validPlayer(pdata) && player.habIsInZone(ZoneID, Index) && vector.Distance(player.GetPosition(), getVector()) <= KillRadius && KillRadius != 0)
			{
				//Player Entered Kill Zone Kill Player if warning has been given
				player.leftZone(0); //Removed from all zones
				habPrint("Killed Player: " + player.GetIdentity().GetName() + " ("+player.GetIdentity().GetPlainId()+") for Entering Zone: " + Name, "Always");
				if ( GetHeroesAndBanditsSettings().DebugLogs ){
					player.m_HeroesAndBandits_InZones.Debug();
				}
				if ( OverrideSafeZone )
				{
					player.SetAllowDamage(true);
				}
				FireWeaponClosestGuard(player.GetPosition());
				player.SetHealth(0.0);
			}
			else if (!validPlayer(pdata) && !player.habIsInZone(ZoneID, Index) && vector.Distance(player.GetPosition(), getVector()) <= Radius)
			{
				//Player Entered Warning Zone Issue Warning
				player.enteredZone(ZoneID, Index);
				if ( GetHeroesAndBanditsSettings().DebugLogs ){
					player.m_HeroesAndBandits_InZones.Debug();
				}
				habPrint("Player: " + player.GetIdentity().GetName() + " ("+player.GetIdentity().GetPlainId()+") Entered: " + Name, "Verbose");
				if ( ShowWarningMsg )
				{
					GetHeroesAndBandits().WarnPlayer(Name, WarningMessage, player.GetIdentity().GetPlainId());
				}		
			}
			else if (vector.Distance(player.GetPosition(), getVector()) > Radius && player.habIsInZone(ZoneID, Index))
			{
				if ( GodModPlayers )
				{
					player.SetAllowDamage(true);
				}
				//Player Left Warning Radius
				habPrint("Player: " + player.GetIdentity().GetName() + " ("+player.GetIdentity().GetPlainId()+") Left: " + Name, "Verbose");
				player.leftZone(Index);		
				if ( GetHeroesAndBanditsSettings().DebugLogs ){
					player.m_HeroesAndBandits_InZones.Debug();
				}	
			}
			if ( SubZones && player.habIsInZone(ZoneID, Index) )
			{ 
				for (int subI = 0; subI < SubZones.Count(); subI++)
				{
					SubZones.Get(subI).CheckPlayer(player);
				}
			}
		}
		
	}
	
    void HeroesAndBanditsZone(string name, float x, float z) 
	{
		Name = name;
		X = x;
		Z = z;
    }
	
	vector getVector(){
		return Vector( X, GetGame().SurfaceY(X, Z), Z );
	}
	
	bool validHumanity(float humanity){
			if ( MinHumanity != -1 && MaxHumanity != -1 && humanity >= MinHumanity && humanity <= MaxHumanity){
				return true;
			}else if (MinHumanity == -1 && MaxHumanity == -1){
				return true;
			}else if (MinHumanity == -1 && MaxHumanity != -1 && humanity <= MaxHumanity){
				return true;
			}else if (MinHumanity != -1 && MaxHumanity == -1 && humanity >= MinHumanity){
				return true;
			}
		return false;
	}
	
	bool validPlayer(HeroesAndBanditsPlayer pdata){
		
		if (GetHeroesAndBanditsSettings().Mode != 1 && Affinities.Count() > 0){
			string affinity = pdata.getAffinity().Name;
			for (int i = 0; i < Affinities.Count(); i++){
				if (Affinities.Get(i).Check(pdata.getAffinityPoints(affinity) , affinity))
				{
					return true;
				}
			}
		} 
		if (GetHeroesAndBanditsSettings().Mode != 0 && Affinities.Count() > 0){
			for (int j = 0; j < pdata.Affinities.Count(); j++){
				if ( ( pdata.Affinities.Get(j).Name == "bandit" || pdata.Affinities.Get(j).Name == "hero" ) && GetHeroesAndBanditsSettings().Mode == 2 ){
					//Skipping for Hero and Bandit as it was checked above if in mixed mode
				} else {
					for (int k = 0; k < Affinities.Count(); k++){
						if ( Affinities.Get(i).Check(pdata.Affinities.Get(j).Points,  pdata.Affinities.Get(j).Name)){
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	
	void FireWeaponClosestGuard(vector playerPostion)
	{
		if (!Guards)//If no guards defined exit
		{
			return;
		}
		int closestGuardIndex = -1;
		float closestGuardDistance = 600;
		for ( int i = 0; i < Guards.Count(); i++ )
		{	
			float currentGuardDistance = vector.Distance( Guards.Get(i).getVector(), playerPostion);
			if ( currentGuardDistance < closestGuardDistance)
			{
				closestGuardIndex = i;
				closestGuardDistance = currentGuardDistance;
			}
		}
		if ( closestGuardIndex == -1 ){
			return;
		} else {
			Guards.Get(closestGuardIndex).FireWeapon();
		} 
	}
	
	HeroesAndBanditsZone GetChild( array< int > inZones){
		int maxIndex = inZones.Count() - 1;
		if (maxIndex == Index){
			return this;
		} else {
			return SubZones.Get(inZones.Get(Index + 1)).GetChild( inZones );
		}
	}
}