#ifndef INCLUDE_AL_PANNING_VBAP
#define INCLUDE_AL_PANNING_VBAP

#include "allocore/sound/al_AudioScene.hpp"

#define MAX_NUM_VBAP_TRIPLETS 512
#define MIN_VOLUME_TO_LENGTH_RATIO 0.01
#define MIN_LENGTH 0.00001



namespace al{

struct SpeakerTriple{
	int s1;
	Vec3d s1Vec;
	int s2;
	Vec3d s2Vec;
	int s3;
	Vec3d s3Vec;
	Vec3d vec[3];
	Mat3d mat;

	void loadVectors(std::vector<Speaker>& spkrs){
		s1Vec = spkrs[s1].vec();
		s2Vec = spkrs[s2].vec();
		if(s3!=-1){
			s3Vec = spkrs[s3].vec();
		}
		vec[0]=s1Vec;
		vec[1]=s2Vec;
		vec[2]=s3Vec;

		mat.set(s1Vec[0],s1Vec[1],s1Vec[2],
				s2Vec[0],s2Vec[1],s2Vec[2],
				s3Vec[0],s3Vec[1],s3Vec[2]
				);
	}


};

class Vbap : public Spatializer{
private:
	std::vector<SpeakerTriple> mTriplets;
	unsigned mNumTriplets;
	Listener* mListener;
	unsigned int mCachedTripletIndex;
	bool is3D;
public:

	void dump() {
		printf("Number of Triplets: %d\n",mNumTriplets);
		for (unsigned i = 0; i < mNumTriplets; i++) {
			printf("Triple #%d: %d,%d,%d \n",i,mTriplets[i].s1,mTriplets[i].s2,mTriplets[i].s3);
		}
	}

	void addTriple(SpeakerTriple& st) {
		mTriplets.push_back(st);
		++mNumTriplets;
	}


	Vec3d computeGains(Vec3d& vecA, SpeakerTriple& speak) {
		Mat3d mat = speak.mat;
		unsigned dimensions = is3D?3:2;
		Vec3d vec(0.0,0.0,0.0);
		for (unsigned i = 0; i < dimensions; i++)  // for each node, insert speaker coordinates in matrix
			for (unsigned j = 0; j < dimensions; j++)  // for each node, insert speaker coordinates in matrix
				vec[i] += vecA[j] * mat(j,i);
		//printf("Gains: (%f,%f,%f)",vec[0],vec[1],vec[2]);
		//printf("Triplet: (%d,%d,%d)",speak.s1,speak.s2,speak.s3);
		return vec;
	}


	// 2D VBAP, find pairs of speakers.
	void findSpeakerPairs(std::vector<Speaker>& spkrs){

		unsigned numSpeakers = spkrs.size();
		unsigned j, index;
		unsigned speakerMapping[numSpeakers]; // To map unordered speakers into an ordered set.
		float speakerAngles[numSpeakers];
		float indexAngle;

		// Build a map to the speakers, that points to speaker indexes.
		for (unsigned i = 0; i < numSpeakers; i++) {
			speakerAngles[i] = spkrs[i].azimuth;
			speakerMapping[i] = i;
		}

		// Sort speakers into the map
		for (unsigned i = 1; i < numSpeakers; i++) {
			// Only sort speakers that have elevation == 0. Ignore all other.
			if (spkrs[i].elevation == 0) {

				indexAngle = speakerAngles[i];
				index = speakerMapping[i];
				j = i;

				while ((j > 0) && (speakerAngles[j-1] > indexAngle)) {
					speakerAngles[j] = speakerAngles[j-1];
					speakerMapping[j] = speakerMapping[j-1];
					j = j - 1;

				}
				speakerAngles[j] = indexAngle;
				speakerMapping[j] = index;
			}
		}

		// Add speaker-pairs
		for (unsigned i = 1; i < numSpeakers; i++){
			SpeakerTriple triple;
			triple.s1 = speakerMapping[i-1];
			triple.s2 = speakerMapping[i];
			triple.s3 = -1;
			triple.loadVectors(spkrs);
			addTriple(triple);
		}
		// Add the last speaker-pair
		SpeakerTriple triple;
		triple.s1 = speakerMapping[numSpeakers-1];
		triple.s2 = speakerMapping[0];
		triple.s3 = -1;
		triple.loadVectors(spkrs);
		addTriple(triple);


	}

	bool isCrossing(Vec3d c, Vec3d v, SpeakerTriple trip){
		double a1 = angle(c,trip.s1Vec)+angle(c,trip.s2Vec);
		double a2 = angle(trip.s1Vec,trip.s2Vec);
		double a3 = angle(c,trip.s3Vec)+angle(c,v);
		double a4 = angle(trip.s3Vec,v);
		return (a1==a2 && a3==a4);
	}

	// 3D VBAP, find triplets.
	void findSpeakerTriplets(std::vector<Speaker>& spkrs){
		std::list<SpeakerTriple> triplets;

		unsigned numSpeakers = spkrs.size();
		int numSpeakersSigned = (int)numSpeakers;

		// form all possible triples
		for (unsigned i = 0; i < numSpeakers; i++){
			for (unsigned j = i+1; j < numSpeakers; j++){
				for (unsigned k = j+1; k < numSpeakers; k++){
					SpeakerTriple triplet;
					triplet.s1=i;
					triplet.s2=j;
					triplet.s3=k;
					triplet.loadVectors(spkrs);
					triplets.push_back(triplet);
				}
			}
		}
		printf("Speaker-count=%d, Initial triplet-count=%d\n",numSpeakers,(unsigned)triplets.size());


		// remove too narrow triples
		for(std::list<SpeakerTriple>::iterator it = triplets.begin(); it != triplets.end();++it){
			SpeakerTriple trip = (*it);

			Vec3d xprod = cross(trip.s1Vec,trip.s2Vec);
			float volume = fabs(xprod.dot(trip.s3Vec));
			float length = fabs(angle(trip.s1Vec , trip.s2Vec) ) + fabs(angle(trip.s1Vec , trip.s3Vec) ) + fabs(angle(trip.s2Vec , trip.s3Vec) );
			float ratio;

			if (length > MIN_LENGTH){
				ratio =  volume / length;
			}else{
				ratio = 0.0;
			}

			if (ratio < MIN_VOLUME_TO_LENGTH_RATIO) {
				//printf("v=%f, l=%f, r=%f x=(%f,%f,%f)\n",volume,length,ratio,xprod[0],xprod[1],xprod[2]);
				triplets.erase(it);
				--it;
			}
		}


		for(std::list<SpeakerTriple>::iterator it = triplets.begin(); it != triplets.end();++it){
			SpeakerTriple trip = (*it);
			bool remove = false;
			for(std::list<SpeakerTriple>::iterator it2 = triplets.begin(); it2 != triplets.end();++it2){
				SpeakerTriple trip2 = (*it2);
				for (unsigned j = 0; j < 3; ++j) {
					Vec3d v = trip2.vec[j];
					Vec3d c = cross(cross(trip.s1Vec, trip.s2Vec),cross(trip.s3Vec,v));
					if (isCrossing(c,v,trip) || isCrossing(-c,v, trip)) {
						remove = true;
						printf("Removing v=(%f,%f,%f) c=(%f,%f,%f)\n",v[0],v[1],v[2],c[0],c[1],c[2]);
						break;
					}
				}
			}

			if (remove) {
				triplets.erase(it);
				--it;
			}
		}




		// remove triangles that contain other Speakers
		for(std::list<SpeakerTriple>::iterator it = triplets.begin(); it != triplets.end();++it){
			SpeakerTriple trip = (*it);
			Mat3d invMat = Mat3d(trip.mat).transpose();

			for (int jj = 0; jj < numSpeakersSigned; ++jj) {
				// check to see if the current speaker is one of the nodes of the triple
				if ((jj == trip.s1) || (jj == trip.s2) || (jj == trip.s3) )
					continue;

				Vec3d sVec = spkrs[jj].vec();
				Vec3d v = sVec * invMat;

				// inside if positive or negative near zero, -1e-4 is a magic number
				bool x_inside = (v[0] >= -1e-4);
				bool y_inside = (v[1] >= -1e-4);
				bool z_inside = (v[2] >= -1e-4);


				if ((x_inside) && (y_inside) && ((is3D) || (z_inside) ) ) 	{
					//printf("Removing v=(%f,%f,%f)\n",v[0],v[1],v[2]);
					triplets.erase(it);
					--it;
					break;
				}
			}
		}


		for(std::list<SpeakerTriple>::iterator it = triplets.begin(); it != triplets.end(); ++it) {
			addTriple(*it);
		}

		return;
	}


	Vbap(SpeakerLayout &sl)
	: Spatializer(sl), mCachedTripletIndex(0),is3D(true)
	{}

	void compile(Listener& listener){
		this->mListener=&listener;
		//Check if 3D...
		if(is3D){
			printf("Finding triplets\n");
			findSpeakerTriplets(mSpeakers);
		}else{
			printf("Finding pairs\n");
			findSpeakerPairs(mSpeakers);
		}

		dump();

		if (mNumTriplets == 0 ){
			printf("No SpeakerSets found. Check mode setting or speaker layout.\n");
			throw -1;
		}

	}


	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){
		unsigned currentTripletIndex = mCachedTripletIndex; // Cached source placement, so it starts searching from there.

		Vec3d vec = Vec3d(relpos);
		//printf("(%f,%f,%f)\n",vec[0],vec[1],vec[2]);

		//Rotate vector according to listener-rotation
		Quatd srcRot = this->mListener->pose().quat();
		vec = srcRot.rotate(vec);

		//Silent by default
		Vec3d gains;
		Vec3d gainsTemp;

		// Search thru the triplets array in search of a match for the source position.
		for (unsigned count = 0; count < mNumTriplets; ++count) {
			gainsTemp = computeGains(vec, mTriplets[currentTripletIndex]);
			if ((gainsTemp[0] >= 0) && (gainsTemp[1] >= 0) && (!is3D || (gainsTemp[2] >= 0)) ){
				//printf("Gainstemp: (%f,%f,%f)\n",gainsTemp[0],gainsTemp[1],gainsTemp[2]);
				gainsTemp.normalize();
				gains = gainsTemp*sample/relpos.mag();
				//printf("Found: (%d,%d,%d) \n", mTriplets[currentTripletIndex].s1, mTriplets[currentTripletIndex].s2, mTriplets[currentTripletIndex].s3);
				break;
			}

			++currentTripletIndex;
			if (currentTripletIndex >= mNumTriplets){
				currentTripletIndex = 0;
			}
			//printf("Index: %d\n",count);
		}
		//printf("Gains: (%f,%f,%f)\n",gains[0],gains[1],gains[2]);

		SpeakerTriple triple = mTriplets[currentTripletIndex];

		if(mCachedTripletIndex!=currentTripletIndex){
			printf("Triple: (%d,%d,%d)\n",triple.s1,triple.s2,triple.s3);
			printf("Gains: (%f,%f,%f)\n",gains[0],gains[1],gains[2]);
			printf("Gains-temp: (%f,%f,%f)\n",gainsTemp[0],gainsTemp[1],gainsTemp[2]);
		}

		mCachedTripletIndex = currentTripletIndex; // Store the new index

		io.out(triple.s1,frameIndex) += gains[0];
		io.out(triple.s2,frameIndex) += gains[1];
		if(is3D){
			io.out(triple.s3,frameIndex) += gains[2];
		}
	}


};






} // al::

#endif
